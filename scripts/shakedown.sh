#!/bin/bash

#   Copyright (c) 2026 Juan Minor
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.

# -----------------------------------------------------------------------------
# 20-minute production-shaped shakedown for a multi-GPU box (8x4090 target).
#
# Answers, with one timed run, the questions that decide the real training run:
#   1. How many instances per GPU yield the most games/hour? (1, 2, 3 and 4
#      instances race on different GPUs under full production CPU load.)
#   2. What does co-locating train.py on GPU 0 cost? (GPU 0 runs the same
#      2-instance layout as GPUs 2/4/5/6, plus live training.)
#   3. Does the full pipeline hold up end to end? All workers write into the
#      SHARED data/selfplay directory, train.py consumes it, re-exports ONNX,
#      and every worker hot-reloads the fresh model mid-run.
#   4. Is anything silently on CPU or misconfigured? (Log scan at the end.)
#
# Prerequisites (run from the repo root on the pod):
#   - Release build exists:  ./run_selfplay.sh builds it, or build manually
#   - Auto-tuned config:     rm -f fenrir.cfg && CUDA_VISIBLE_DEVICES=0 ./bin/fenrir --auto-tune
#   - Python deps:           pip install -r training/requirements.txt
#
# Usage:
#   ./scripts/shakedown.sh                 # 20 minutes, default layout
#   DURATION=600 ./scripts/shakedown.sh    # shorter run
#
# Each worker runs in its own working directory so it can have its own
# SearchThreads in fenrir.cfg, while onnx/ and data/selfplay are symlinked
# back to the repo root so the model and training data stay shared.
# -----------------------------------------------------------------------------

set -u

DURATION=${DURATION:-1200}
SIMULATIONS=${SIMULATIONS:-800}

# Layout under test: "gpu:instances:threads_per_instance"
# Total search threads: 10+10+10+12+10+10+10+12 = 84, plus one NN worker
# thread per instance (17) and train.py -> ~full load on 96 vCPUs.
LAYOUT=${LAYOUT:-"0:2:5 1:1:10 2:2:5 3:3:4 4:2:5 5:2:5 6:2:5 7:4:3"}

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
RUN_DIR="$ROOT/shakedown"
cd "$ROOT"

if [ ! -x "$ROOT/bin/fenrir" ]; then
    echo "ERROR: $ROOT/bin/fenrir not found. Build first (see run_selfplay.sh)."
    exit 1
fi

if [ ! -f "$ROOT/fenrir.cfg" ]; then
    echo "ERROR: fenrir.cfg not found. Run the auto-tuner first:"
    echo "  rm -f fenrir.cfg && CUDA_VISIBLE_DEVICES=0 ./bin/fenrir --auto-tune"
    exit 1
fi

# Refuse layouts that reference GPUs this machine doesn't have: those
# workers would silently fall back to CPU and corrupt the measurement.
if command -v nvidia-smi >/dev/null 2>&1; then
    GPU_COUNT=$(nvidia-smi --list-gpus 2>/dev/null | wc -l)
    if [ "$GPU_COUNT" -gt 0 ]; then
        for SPEC in $LAYOUT; do
            GPU=${SPEC%%:*}
            if [ "$GPU" -ge "$GPU_COUNT" ]; then
                echo "ERROR: layout references GPU $GPU but only $GPU_COUNT GPU(s) present."
                echo "Override the default 8-GPU layout, e.g.:"
                echo "  LAYOUT=\"0:2:12\" $0"
                exit 1
            fi
        done
    fi
fi

BATCH_SIZE=$(grep -oP '^BatchSize=\K[0-9]+' fenrir.cfg || echo 256)
PIPELINE_TARGET=$(grep -oP '^PipelineTarget=\K[0-9]+' fenrir.cfg || echo 512)
BATCH_TIMEOUT_MS=$(grep -oP '^BatchTimeoutMs=\K[0-9]+' fenrir.cfg || echo 4)

export LD_LIBRARY_PATH=$ROOT/build/_deps/onnxruntime-src/lib:${LD_LIBRARY_PATH:-}
export LD_LIBRARY_PATH=/usr/local/lib/python3.12/dist-packages/nvidia/cudnn/lib:$LD_LIBRARY_PATH

rm -rf "$RUN_DIR"
mkdir -p "$RUN_DIR" "$ROOT/data/selfplay"

PIDS=()
cleanup() {
    echo "Stopping all shakedown processes..."
    pkill -f "fenrir --selfplay" 2>/dev/null
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null
    done
}
trap cleanup INT TERM

echo "=== Fenrir shakedown: ${DURATION}s, ${SIMULATIONS} sims/move ==="
echo "Tuned params: BatchSize=$BATCH_SIZE PipelineTarget=$PIPELINE_TARGET BatchTimeoutMs=$BATCH_TIMEOUT_MS"
echo "Layout: $LAYOUT"
echo ""

TOTAL_THREADS=0
for SPEC in $LAYOUT; do
    GPU=${SPEC%%:*}
    REST=${SPEC#*:}
    INSTANCES=${REST%%:*}
    THREADS=${REST#*:}

    for INST in $(seq 1 "$INSTANCES"); do
        WDIR="$RUN_DIR/gpu${GPU}_i${INST}"
        mkdir -p "$WDIR/logs" "$WDIR/data"
        ln -sfn "$ROOT/onnx" "$WDIR/onnx"
        ln -sfn "$ROOT/data/selfplay" "$WDIR/data/selfplay"

        {
            echo "[Tuning]"
            echo "SearchThreads=$THREADS"
            echo "BatchSize=$BATCH_SIZE"
            echo "PipelineTarget=$PIPELINE_TARGET"
            echo "BatchTimeoutMs=$BATCH_TIMEOUT_MS"
        } > "$WDIR/fenrir.cfg"

        # stdbuf: line-buffer stdout so per-game log lines survive the
        # end-of-run pkill (C++ won't flush block buffers on SIGTERM).
        (cd "$WDIR" && env CUDA_VISIBLE_DEVICES=$GPU stdbuf -oL "$ROOT/bin/fenrir" --selfplay \
            --gpu-id 0 --simulations "$SIMULATIONS" --games 1000000 \
            > logs/selfplay.log 2>&1) &
        disown
        TOTAL_THREADS=$((TOTAL_THREADS + THREADS))
    done
    echo "GPU $GPU: $INSTANCES instance(s) x $THREADS threads"
done
echo "Total search threads: $TOTAL_THREADS"
echo ""

echo "Starting train.py on GPU 0..."
(cd "$ROOT" && env CUDA_VISIBLE_DEVICES=0 python3 training/train.py \
    > "$RUN_DIR/train.log" 2>&1) &
PIDS+=($!)

if command -v nvidia-smi >/dev/null 2>&1; then
    nvidia-smi --query-gpu=index,utilization.gpu --format=csv,noheader,nounits -l 15 \
        > "$RUN_DIR/gpu_util.csv" 2>/dev/null &
    PIDS+=($!)
fi

echo "Running for $DURATION seconds... (watch -n 2 nvidia-smi in another shell)"
sleep "$DURATION"
cleanup
sleep 3

# -----------------------------------------------------------------------------
# Report
# -----------------------------------------------------------------------------
echo ""
echo "==================== SHAKEDOWN REPORT ===================="
printf "%-6s %-10s %-9s %-8s %-10s %-9s\n" "GPU" "layout" "games" "g/hr" "avg-moves" "util%"

BEST_RATE=0
BEST_LAYOUT=""
for SPEC in $LAYOUT; do
    GPU=${SPEC%%:*}
    REST=${SPEC#*:}
    INSTANCES=${REST%%:*}
    THREADS=${REST#*:}

    GAMES=$(cat "$RUN_DIR/gpu${GPU}"_i*/logs/selfplay.log 2>/dev/null | grep -c "finished in")
    AVG_MOVES=$(cat "$RUN_DIR/gpu${GPU}"_i*/logs/selfplay.log 2>/dev/null \
        | grep -oP 'finished in \K[0-9]+' \
        | awk '{s+=$1; n++} END { if (n>0) printf "%.0f", s/n; else print "-" }')
    RATE=$(awk -v g="$GAMES" -v d="$DURATION" 'BEGIN { printf "%.0f", g*3600/d }')
    UTIL=$(awk -F', *' -v gpu="$GPU" '$1==gpu {s+=$2; n++} END { if (n>0) printf "%.0f", s/n; else print "-" }' \
        "$RUN_DIR/gpu_util.csv" 2>/dev/null || echo "-")

    NOTE=""
    [ "$GPU" = "0" ] && NOTE="(+train.py)"
    printf "%-6s %-10s %-9s %-8s %-10s %-9s %s\n" "$GPU" "${INSTANCES}x${THREADS}t" "$GAMES" "$RATE" "$AVG_MOVES" "$UTIL" "$NOTE"

    if [ "$GPU" != "0" ] && [ "$RATE" -gt "$BEST_RATE" ]; then
        BEST_RATE=$RATE
        BEST_LAYOUT="${INSTANCES} instance(s) x ${THREADS} threads"
    fi
done

echo ""
echo "Best per-GPU layout (excluding GPU 0): $BEST_LAYOUT at ~$BEST_RATE games/hr/GPU"
echo "Fleet estimate at that layout: ~$((BEST_RATE * 8)) games/hr across 8 GPUs"
echo ""

echo "--- Training pipeline ---"
CYCLES=$(grep -c "Training cycle finished" "$RUN_DIR/train.log" 2>/dev/null)
CYCLES=${CYCLES:-0}
EXPORTS=$(grep -c "Exported updated weights" "$RUN_DIR/train.log" 2>/dev/null)
EXPORTS=${EXPORTS:-0}
BACKLOG=$(ls "$ROOT/data/selfplay" 2>/dev/null | grep -c "READY_")
echo "Training cycles completed: $CYCLES"
echo "Model exports (hot-reloaded by workers): $EXPORTS"
echo "Unconsumed READY files left: $BACKLOG (should be small if train.py keeps up)"
echo ""

echo "--- Health checks ---"
FAIL=0
if grep -rq "Falling back to CPU" "$RUN_DIR"/gpu*_i*/logs/ 2>/dev/null; then
    echo "FAIL: at least one worker fell back to CPU inference:"
    grep -rl "Falling back to CPU" "$RUN_DIR"/gpu*_i*/logs/
    FAIL=1
else
    echo "OK: no CPU fallbacks."
fi
if grep -rq "unexpected tensor size" "$RUN_DIR"/gpu*_i*/logs/ 2>/dev/null; then
    echo "FAIL: policy tensor size mismatch — the deployed onnx model is stale."
    FAIL=1
else
    echo "OK: no tensor size mismatches."
fi
if [ "$CYCLES" -eq 0 ]; then
    echo "WARN: train.py completed no cycles — check $RUN_DIR/train.log"
fi
echo "==========================================================="
[ "$FAIL" -eq 0 ] && echo "Shakedown PASSED." || echo "Shakedown FAILED — fix the issues above before the long run."
exit "$FAIL"
