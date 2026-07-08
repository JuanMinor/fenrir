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

echo "Starting 8 Fenrir instances across 8 GPUs..."

for GPU in {0..7}; do
    echo "Launching Fenrir on GPU $GPU..."
    # We run 1 instance per GPU. Each instance runs 16 threads internally via MCTS.
    # Total threads: 8 * 16 = 128 (Perfect for a 96 vCPU system with Hyperthreading)
    ./bin/fenrir --selfplay --gpu-id $GPU --simulations 1600 --games 30000000 > logs/gpu${GPU}.log 2>&1 &
done

echo "All 8 instances have been launched in the background!"
echo "To monitor progress, run: tail -f logs/gpu0.log"
echo "To stop them, run: pkill fenrir"
