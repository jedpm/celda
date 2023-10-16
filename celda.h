#ifndef CELDA_CELDA_H
#define CELDA_CELDA_H
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define CELDA_ERROR(s) 			do { fprintf(stderr, "celda: abort: %s.\n", s); exit(EXIT_FAILURE); } while (0)
#define CELDA_WARNG(s, ...) 		fprintf(stderr, "celda: warng: " s ".\n", ##__VA_ARGS__)
#define CELDA_CHECK_MEM(p)		do { if (!p) { CELDA_ERROR("no available memory to allocate what is needed"); } } while (0)
#define CELDA_IS_LIT(k)			(k >= 1 && k <= 3)
#define CELDA_IS_CNST(k)		(k == 1 || k == 2 || k == -1)
#define CELDA_IS_DOUBLE_FORMED(k)	(k >= 18)
#define CELDA_IS_MATH_SYMBOL(k)		(k >= 6 && k <= 13)

#define CELDA_TOKEN_MAX_LEN	64
#define CELDA_TOKEN_PER_EXP	16
#define CELDA_SUB_EXP_PER_EXP	8

typedef enum Token_Type {
	type_error = -1,
	type_unknown,

	type_string, // 1
	type_number,
	type_reference,

	type_arithmetic, // 4
	type_condition,

	type_add, // 6
	type_sub,
	type_mul,
	type_div,
	type_mod,
	type_pow,

	type_left_p, // 12
	type_rigth_p,
	type_left_c,
	type_rigth_c,

	type_greater, // 16
	type_less,
	type_grequ,
	type_leequ,
	type_equals,
	type_nequal
} Token_Type;

typedef struct Token {
	char token[CELDA_TOKEN_MAX_LEN];
	Token_Type type;
} Token;

typedef struct Expression {
	Token tokens[CELDA_TOKEN_PER_EXP];
	struct Expression* children, *parent;
	uint16_t child_i, token_i;
} Expr;

typedef struct Cell {
	Expr expression;
	char cell[CELDA_TOKEN_MAX_LEN];
	Token_Type type;
	bool first;
} Cell;

typedef struct Spread {
	Cell* cells;
	uint16_t* firsts;
	uint16_t cells_i, first_i;
	bool is_first;
} Spread;

#endif

