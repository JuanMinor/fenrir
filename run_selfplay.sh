#!/bin/bash
# Multi-GPU Self-Play Launcher for Fenrir
# This script spins up 8 instances of Fenrir (one for each GPU)

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

echo "Starting 48 Fenrir instances across 8 GPUs (6 instances per GPU to maximize utilization)..."

for GPU in {0..7}; do
    for INSTANCE in {1..6}; do
        echo "Launching Fenrir instance $INSTANCE on GPU $GPU..."
        env CUDA_VISIBLE_DEVICES=$GPU ./bin/fenrir --selfplay --gpu-id 0 --simulations 400 --games 30000000 > logs/gpu${GPU}_inst${INSTANCE}.log 2>&1 &
    done
done

echo "All 48 instances have been launched in the background!"
echo "To monitor progress, run: tail -f logs/gpu0_inst1.log"
echo "To stop them, run: pkill fenrir"
