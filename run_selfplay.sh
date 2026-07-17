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

echo "Starting 16 Fenrir instances across 8 GPUs (2 instances per GPU to maximize utilization)..."

for GPU in {1..7}; do
    echo "Launching single instance of Fenrir to run on $GPU..."
    env CUDA_VISIBLE_DEVICES=$GPU ./bin/fenrir --selfplay --gpu-id $GPU --simulations 50000 --games 500000 > logs/gpu_$GPU.log 2>&1 &
done

echo "All 16 instances have been launched in the background!"
echo "To monitor progress, run: tail -f logs/gpu0_inst1.log"
echo "To stop them, run: pkill fenrir"
