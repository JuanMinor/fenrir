#include "include/abi/c_api.h"
#include "include/engine/engine.h"
#include <string>
#include <cstring>
#include <stdexcept>

extern "C" {
    void* fenrir_create() {
        return new fenrir::Engine();
    }

    void fenrir_destroy(void* engine) {
        delete static_cast<fenrir::Engine*>(engine);
    }

    bool fenrir_make_move(void* engine, const char* from, const char* to) {
        auto* eng = static_cast<fenrir::Engine*>(engine);
        try {
            eng->make_move(fenrir::Move(from, to));
            return true;
        } catch (const std::invalid_argument&) {
            return false;
        }
    }

    int fenrir_generate_all_moves(void* engine, char* out_buffer, size_t max_len) {
        auto* eng = static_cast<fenrir::Engine*>(engine);
        auto moves = eng->generate_all_moves();
        std::string result = "";
        for (const auto& m : moves) {
            result += m.to_uci_notation() + ",";
        }
        if (!result.empty()) {
            result.pop_back(); // remove last comma
        }
#ifdef _MSC_VER
        strncpy_s(out_buffer, max_len, result.c_str(), max_len - 1);
#else
        strncpy(out_buffer, result.c_str(), max_len - 1);
        out_buffer[max_len - 1] = '\0';
#endif
        return static_cast<int>(moves.size());
    }

    void fenrir_get_fen(void* engine, char* out_buffer, size_t max_len) {
        auto* eng = static_cast<fenrir::Engine*>(engine);
        std::string fen = eng->get_fen();
#ifdef _MSC_VER
        strncpy_s(out_buffer, max_len, fen.c_str(), max_len - 1);
#else
        strncpy(out_buffer, fen.c_str(), max_len - 1);
        out_buffer[max_len - 1] = '\0';
#endif
    }

    void fenrir_print_board(void* engine) {
        auto* eng = static_cast<fenrir::Engine*>(engine);
        eng->print_board();
    }
}
