#include "arith.h"
#include <math.h>
#define EXPRESSION_SIZE		74
#define STACK_STARTS_AT		64

typedef struct Value {
	double asnum;
	Token_Type asopt;
} Val;

typedef struct Arithmetic {
	Val values[EXPRESSION_SIZE];
	uint16_t expr_i, stck_i;
	uint16_t parentheses;
} Arith;

static Arith g_expression;

static bool push_at_beginning (const char*, const Token_Type);
static bool push_at_stack (const Token_Type);
static bool need_2_exchange (const Token_Type, const Token_Type);
static bool end_of_parentheses ();
static void solve (double*, double, const Token_Type);

void arith_init ()
{
	memset(&g_expression, 0, sizeof(Arith));
	g_expression.stck_i = STACK_STARTS_AT;
	g_expression.expr_i = 0;
}

bool arith_push (const char* as, const Token_Type is)
{
	if (is == type_number)
		return push_at_beginning(as, is);
	return push_at_stack(is);
}

bool arith_solve (char* ans)
{
	uint16_t i;
	for (i = g_expression.stck_i - 1; i >= STACK_STARTS_AT; i--)
		push_at_beginning(NULL, g_expression.values[i].asopt);

	double numstack[3] = {0};
	uint16_t nums_i = 0;

	for (i = 0; i < g_expression.expr_i; i++) {
		Val *v = &g_expression.values[i];
		if (v->asopt == type_number) {
			numstack[nums_i++] = v->asnum;
		}
		else {
			if (nums_i <= 1)
				return false;
			solve(&numstack[nums_i - 2], numstack[nums_i - 1], v->asopt);
			nums_i--;
		}
	}

	snprintf(ans, CELDA_TOKEN_MAX_LEN, "%3.f", numstack[0]);
	return true;
}

static bool push_at_beginning (const char* as, const Token_Type is)
{
	if (g_expression.expr_i == STACK_STARTS_AT)
		return false;

	Val value = {	
		.asnum = (as) ? atol(as) : 0,
		.asopt = is
	};

	g_expression.values[g_expression.expr_i++] = value;
	return true;
}

static bool push_at_stack (const Token_Type is)
{
	if (g_expression.stck_i == EXPRESSION_SIZE)
		return false;

	if (is == type_rigth_p)
		return end_of_parentheses();

	if (is == type_left_p)
		g_expression.parentheses++;

	Val value = { .asopt = is };
	if (g_expression.stck_i == STACK_STARTS_AT)
		goto only_push;

	uint16_t top_i = g_expression.stck_i - 1;
	Token_Type top = g_expression.values[top_i].asopt;

	while (need_2_exchange(is, top)) {
		push_at_beginning(NULL, top);
		g_expression.stck_i = top_i;

		if (top_i == STACK_STARTS_AT) break;
		top = g_expression.values[--top_i].asopt;
	}

	only_push:
	g_expression.values[g_expression.stck_i++] = value;
	return true;
}

static bool need_2_exchange (const Token_Type in, const Token_Type top)
{
	if (in == type_left_p || top == type_left_p)
		return false;

	static const uint16_t sames[] = {
		type_add * type_sub,
		type_mul * type_div,
		type_mul * type_mod,
		type_mod * type_div
	};

	uint16_t a = in * top;
	bool same = (a == sames[0]) || (a == sames[1]) ||
		    (a == sames[2]) || (a == sames[3]);

	if (same || in == top)
		return true;
	return in < top;
}

static bool end_of_parentheses ()
{
	if (!g_expression.parentheses)
		return false;

	uint16_t pos = --g_expression.stck_i;
	while (g_expression.values[pos].asopt != type_left_p)
		push_at_beginning(NULL, g_expression.values[pos--].asopt);

	g_expression.stck_i = pos;
	return true;
}

static void solve (double *a, double b, const Token_Type op_)
{
	switch (op_) {
		case type_add: *a = *a + b; break;
		case type_sub: *a = *a - b; break;
		case type_mul: *a = *a * b; break;
		case type_div: *a = *a / b; break;
		case type_mod: *a = fmod(*a, b); break;
		case type_pow: *a = pow(*a, b); break;
	}
}

