#!/bin/bash
# Production launcher for Fenrir training: self-play workers on every GPU
# plus train.py co-located on GPU 0, with periodic weight checkpoints.
#
# Defaults follow the measured production shakedowns (3 instances x 4
# threads per GPU at BatchTimeoutMs=4 won at ~3,300 games/hr per 4090).
# Override via environment:
#   SIMULATIONS=800 INSTANCES_PER_GPU=3 GPUS="0 1 2 3" ./run_selfplay.sh
#   SEARCH_THREADS=4         per-instance threads written into fenrir.cfg
#   TRAIN=0                  skip launching train.py (self-play only)
#   CHECKPOINT_INTERVAL=3600 seconds between fenrir.pth snapshots (0 = off)

# Compile first to ensure we have the latest binary
echo "Building Fenrir with GPU support..."
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
cd ..

echo "Creating directories..."
mkdir -p data/selfplay logs checkpoints

export LD_LIBRARY_PATH=$PWD/build/_deps/onnxruntime-src/lib:${LD_LIBRARY_PATH:-}
# Locate torch's bundled cuDNN wherever pip put it (python version varies by image)
CUDNN_LIB=$(python3 -c "import nvidia.cudnn, os; print(os.path.join(os.path.dirname(nvidia.cudnn.__file__), 'lib'))" 2>/dev/null)
if [ -n "$CUDNN_LIB" ]; then
    export LD_LIBRARY_PATH=$CUDNN_LIB:$LD_LIBRARY_PATH
fi

# Training search: game volume matters far more than per-move depth.
SIMULATIONS=${SIMULATIONS:-800}
GAMES=${GAMES:-1000000}
INSTANCES_PER_GPU=${INSTANCES_PER_GPU:-3}
SEARCH_THREADS=${SEARCH_THREADS:-4}
TRAIN=${TRAIN:-1}
CHECKPOINT_INTERVAL=${CHECKPOINT_INTERVAL:-3600}

# Workers read SearchThreads from fenrir.cfg (cwd); pin it to the layout's
# per-instance thread count so cfg and layout can't drift apart. The other
# cfg values (batch size/pipeline/timeout) come from --auto-tune.
if [ -f fenrir.cfg ]; then
    sed -i "s/^SearchThreads=.*/SearchThreads=${SEARCH_THREADS}/" fenrir.cfg
    echo "fenrir.cfg: SearchThreads pinned to ${SEARCH_THREADS}"
else
    echo "WARNING: fenrir.cfg not found — run 'CUDA_VISIBLE_DEVICES=0 ./bin/fenrir --auto-tune' first for tuned batch parameters."
fi

# Default to every GPU the machine actually has.
if [ -z "${GPUS:-}" ]; then
    if command -v nvidia-smi >/dev/null 2>&1; then
        GPU_COUNT=$(nvidia-smi --list-gpus 2>/dev/null | wc -l)
        GPUS=$(seq 0 $((GPU_COUNT - 1)) | tr '\n' ' ')
    else
        GPUS="0"
    fi
fi

# Sweep strays from a previous/aborted run: an orphaned trainer keeps
# consuming data and overwriting the model alongside the new one.
if pgrep -f "fenrir --selfplay" >/dev/null 2>&1 || pgrep -f "training/train.py" >/dev/null 2>&1; then
    echo "Killing stray fenrir/train.py processes from a previous run..."
    pkill -f "fenrir --selfplay" 2>/dev/null
    pkill -f "training/train.py" 2>/dev/null
    sleep 2
fi

TOTAL=0
for GPU in $GPUS; do
    for INST in $(seq 1 "$INSTANCES_PER_GPU"); do
        echo "Launching Fenrir self-play instance $INST on GPU $GPU..."
        # CUDA_VISIBLE_DEVICES remaps the selected GPU to ordinal 0 inside
        # the process, so --gpu-id must always be 0 here. stdbuf keeps
        # per-game log lines flowing for live monitoring.
        env CUDA_VISIBLE_DEVICES=$GPU stdbuf -oL ./bin/fenrir --selfplay --gpu-id 0 \
            --simulations "$SIMULATIONS" --games "$GAMES" \
            > "logs/gpu${GPU}_inst${INST}.log" 2>&1 &
        TOTAL=$((TOTAL + 1))
    done
done
echo "Launched $TOTAL self-play instances ($INSTANCES_PER_GPU per GPU) on GPUs: $GPUS"

if [ "$TRAIN" = "1" ]; then
    echo "Launching train.py on GPU 0..."
    env CUDA_VISIBLE_DEVICES=0 python3 training/train.py > logs/train.log 2>&1 &
    echo "Trainer PID: $!"
fi

if [ "$CHECKPOINT_INTERVAL" -gt 0 ] 2>/dev/null; then
    (
        while true; do
            sleep "$CHECKPOINT_INTERVAL"
            if [ -f onnx/fenrir.pth ]; then
                cp onnx/fenrir.pth "checkpoints/fenrir_$(date +%Y%m%d_%H%M).pth"
            fi
        done
    ) > /dev/null 2>&1 &
    echo "Checkpointing onnx/fenrir.pth to checkpoints/ every ${CHECKPOINT_INTERVAL}s (PID: $!)"
fi

echo ""
echo "Monitor games:   tail -f logs/gpu1_inst1.log"
echo "Monitor trainer: tail -f logs/train.log"
echo "Monitor GPUs:    watch -n 2 nvidia-smi"
echo "Game rate:       grep -h 'finished in' logs/gpu*_inst*.log | wc -l   (run twice, 60s apart)"
echo "Stop everything: pkill -f 'fenrir --selfplay'; pkill -f training/train.py"
