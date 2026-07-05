#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include "include/engine/engine.h"
#include "include/chess/move.h"

uint64_t perft(fenrir::Engine& engine, int depth) {
    if (depth == 0) return 1ULL;
    
    std::vector<fenrir::Move> moves = engine.generate_all_moves();
    
    if (depth == 1) return moves.size();
    
    uint64_t nodes = 0;
    for (const auto& move : moves) {
        engine.make_move_fast(move);
        nodes += perft(engine, depth - 1);
        engine.undo_move();
    }
    return nodes;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <depth> <fen>" << std::endl;
        return 1;
    }
    
    int depth = std::stoi(argv[1]);
    std::string fen = argv[2];
    for (int i = 3; i < argc; ++i) {
        fen += " ";
        fen += argv[i];
    }
    
    std::cout << "Running Perft depth " << depth << " for FEN: " << fen << std::endl;
    
    try {
        fenrir::Engine engine(fen);
        
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft(engine, depth);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> diff = end - start;
        double seconds = diff.count();
        double nps = static_cast<double>(nodes) / seconds;
        
        std::cout << "Nodes generated: " << nodes << std::endl;
        std::cout << "Time taken: " << seconds << " seconds" << std::endl;
        std::cout << "NPS: " << static_cast<uint64_t>(nps) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
