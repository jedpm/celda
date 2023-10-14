#ifndef CELDA_BUILD_H
#define CELDA_BUILD_H
#include "celda.h"

void build_start (uint16_t, uint16_t);
void build_row ();
void build_cell ();
void build_token (const char*, size_t, const Token_Type);
void build_build ();


#endif
