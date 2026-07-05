# Fenrir Engine Conversion Plan

## Vision
Transform Fenrir from a pure "Move Validation Shared Library" into a world-class **Neural Network Chess Engine** modeled after AlphaZero/Leela Chess Zero.

This requires transitioning to a monolithic architecture:
1. `fenrir` - The standalone C++ engine executable that implements UCI, Monte Carlo Tree Search (MCTS), Neural Network Inference, the highly-parallelized self-play generation loop, and the blazing fast C++ rules and move generation layer.
2. **Local Training Pipeline** - An external Python/PyTorch repository that continuously reads the C++ generated game data from disk, optimizes the neural network via gradient descent, and exports updated weights for the C++ engine to reload.

## Phase 1: MCTS Foundation (v0.4.0)
Instead of Alpha-Beta Minimax, Fenrir will use Monte Carlo Tree Search to explore deep positional lines.
- **Goal:** Implement native C++ MCTS that uses the internal move generation layer.
- **Tasks:**
  - Create `src/search/mcts.h` and `src/search/mcts.cpp`.
  - Implement Node structures (visits, win probability, prior probability).
  - Implement the 4 MCTS phases: Selection (PUCT formula), Expansion, Simulation, Backpropagation.
  - Temporarily use random playouts (or basic material evaluation) for Simulation until the Neural Network is ready.

## Phase 2: Neural Network Inference & Batched Execution (v0.5.0)
Replace the MCTS random playouts with a deep neural network evaluation. Crucially, this must support batched GPU inference to avoid starvation.
- **Goal:** Load trained weights and execute forward passes natively in C++ across multiple threads.
- **Tasks:**
  - Create `src/eval/nn_eval.h`.
  - Integrate a high-performance C++ tensor library (e.g., ONNX Runtime with DirectML or ROCm Execution Provider for AMD GPU support).
  - Implement board-to-tensor state formatting (representing the 8x8 board as input channels).
  - Implement batched inference: C++ MCTS threads queue up leaf nodes (e.g., 256 or 512 states at once) and send them to the GPU simultaneously.

## Phase 3: UCI Protocol & Monolithic Executable (v0.6.0)
Wrap the MCTS engine in a command-line interface that chess GUIs can talk to.
- **Goal:** Build the `fenrir` executable and UCI listener loop.
- **Tasks:**
  - Create `src/uci/uci.cpp` with a `std::cin` listening loop.
  - Implement `position startpos moves ...` by calling `make_move_fast()`.
  - Implement `go depth X` or `go movetime Y` to trigger the MCTS engine.
  - Output `bestmove` when the search completes.
  - Update `CMakeLists.txt` to compile all sources directly into the `fenrir` executable.

## Phase 4: C++ Self-Play & Local Training Pipeline (v0.7.0)
To avoid GPU starvation and Python GIL bottlenecks, C++ must drive the self-play generation. Python is strictly reserved for gradient descent.
- **Goal:** Generate millions of games in C++ and train the model offline in Python.
- **Tasks:**
  - Build a `--self-play` mode into the `fenrir` executable.
  - **Warm Start (Bootstrap):** Do not start training from absolute zero. Supply an initial, basic `.onnx` weights file (e.g., pre-trained on a small dataset of human games) so the engine starts with a baseline competency, drastically reducing local compute time.
  - C++ runs highly parallelized MCTS and plays games against itself, saving game data (states, probabilities, outcomes) to disk as raw binaries or PGNs. The engine should detect the OS: if on Windows, target the `D:\` drive (to utilize the 1TB NVMe capacity); otherwise, fall back to a local relative path (for Linux/macOS dev containers).
  - Write a PyTorch script (in a separate repository) that asynchronously reads these files, runs the optimization loop on the GPU, and writes out a new `.onnx` file.
  - The C++ engine detects the new `.onnx` file, reloads the weights, and continues self-play.

## Architecture Diagram
```
[ Local AMD RX 5700XT GPU / Ryzen 5800X (Windows/MSVC) ]
       |
(PyTorch Training Repo - Python)
  <-- Reads millions of C++ generated games from disk
  --> Writes optimized .onnx weights back to disk
       |
[ fenrir Executable - C++ ]
  |-- Move Generation & Validation
  |-- UCI I/O Loop (For human/GUI play)
  |-- Self-Play Loop (For training generation)
  |-- MCTS Algorithm (Parallelized across 16 CPU threads)
  |-- ONNX/DirectML Batched Inference (Sends 512 states to AMD GPU at once)
```
