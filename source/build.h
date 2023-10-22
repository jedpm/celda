#ifndef CELDA_BUILD_H
#define CELDA_BUILD_H
#include "celda.h"

Spread* build_start (uint16_t, uint16_t);
void build_init_cell (Spread*);
void build_init_row (Spread*);
void build_save_token (Spread*, const char*, size_t, const Token_Type);
void build_solve_this (Spread*);

#endif
