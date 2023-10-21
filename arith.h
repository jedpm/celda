#ifndef CELDA_ARITH_H
#define CELDA_ARITH_H
#define EXPRESSION_SIZE		74
#define STACK_STARTS_AT		64
#include "celda.h"

typedef struct Value {
	double asnum;
	Token_Type asopt;
} Val;

typedef struct Arithmetic {
	Val values[EXPRESSION_SIZE];
	uint16_t expr_i, stck_i;
	uint16_t parentheses;
} Arith;

Arith arith_init ();
bool arith_push (Arith*, const char*, const Token_Type);
bool arith_solve (Arith*, char*);

#endif
