#!/bin/bash
# Multi-GPU Self-Play Launcher for Fenrir
# Launches INSTANCES_PER_GPU self-play processes on each GPU in GPUS.
# Override defaults via environment, e.g.:
#   SIMULATIONS=800 INSTANCES_PER_GPU=2 GPUS="0 1 2 3 4 5 6 7" ./run_selfplay.sh

# Compile first to ensure we have the latest binary
echo "Building Fenrir with GPU support..."
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
cd ..

echo "Creating directories..."
mkdir -p data/selfplay
mkdir -p logs

export LD_LIBRARY_PATH=$PWD/build/_deps/onnxruntime-src/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib/python3.12/dist-packages/nvidia/cudnn/lib:$LD_LIBRARY_PATH

# Training search: game volume matters far more than per-move depth.
# AlphaZero self-play used 800 simulations per move.
SIMULATIONS=${SIMULATIONS:-800}
GAMES=${GAMES:-500000}
INSTANCES_PER_GPU=${INSTANCES_PER_GPU:-2}
# GPU 0 is shared with train.py; its training cycles are short and bursty,
# so co-locating self-play there is a net win. Set GPUS="1 2 3 4 5 6 7"
# to give training a dedicated GPU instead.
GPUS=${GPUS:-"0 1 2 3 4 5 6 7"}

TOTAL=0
for GPU in $GPUS; do
    for INST in $(seq 1 "$INSTANCES_PER_GPU"); do
        echo "Launching Fenrir self-play instance $INST on GPU $GPU..."
        # CUDA_VISIBLE_DEVICES remaps the selected GPU to ordinal 0 inside
        # the process, so --gpu-id must always be 0 here. Passing the real
        # index would target a nonexistent device and fall back to CPU.
        env CUDA_VISIBLE_DEVICES=$GPU ./bin/fenrir --selfplay --gpu-id 0 \
            --simulations "$SIMULATIONS" --games "$GAMES" \
            > "logs/gpu${GPU}_inst${INST}.log" 2>&1 &
        TOTAL=$((TOTAL + 1))
    done
done

echo "Launched $TOTAL instances ($INSTANCES_PER_GPU per GPU) on GPUs: $GPUS"
echo "Monitor progress: tail -f logs/gpu1_inst1.log"
echo "Monitor GPUs:     watch -n 2 nvidia-smi"
echo "Stop everything:  pkill fenrir"
