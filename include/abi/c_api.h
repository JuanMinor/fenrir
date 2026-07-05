#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* fenrir_create();
void fenrir_destroy(void* engine);
bool fenrir_make_move(void* engine, const char* from, const char* to);
int fenrir_generate_all_moves(void* engine, char* out_buffer, size_t max_len);
void fenrir_get_fen(void* engine, char* out_buffer, size_t max_len);
void fenrir_print_board(void* engine);

#ifdef __cplusplus
}
#endif
