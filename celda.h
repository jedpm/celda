#ifndef CELDA_CELDA_H
#define CELDA_CELDA_H
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define CELDA_ERROR(s) 		do { fprintf(stderr, "celda: abort: %s.\n", s); exit(EXIT_FAILURE); } while (0)
#define CELDA_WARNG(s, ...) 	fprintf(stderr, "celda: warng: " s ".\n", ##__VA_ARGS__)
#define CELDA_IS_LIT(k)		(k >= 1 && k <= 3)

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

#endif
