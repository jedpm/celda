#include "arith.h"
#define EXPRESSION_SIZE		74
#define STACK_STARTS_AT		64

typedef struct Value {
	double asnum;
	Token_Type asopt;
} Val;

typedef struct Arithmetic {
	Val values[EXPRESSION_SIZE];
	uint16_t expr_i, stck_i;
} Arith;

static Arith g_expression;

static bool push_at_beginning (const char*, const Token_Type);
static bool push_at_stack (const Token_Type);
static bool exchange (const Token_Type, const Token_Type);

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

void arith_solve ()
{
	uint16_t i;
	for (i = g_expression.stck_i - 1; i >= STACK_STARTS_AT; i--)
		push_at_beginning(NULL, g_expression.values[i].asopt);

	char* x = "+-*/%^";
	for (i = 0; i < g_expression.expr_i; i++) {
		Val *v = &g_expression.values[i];
		if (v->asopt == type_number)
			printf("num: %f\n", v->asnum);
		else
			printf("opt: %c\n", x[v->asopt - type_add]);
	}
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
		return true;

	Val value = { .asopt = is };
	if (g_expression.stck_i == STACK_STARTS_AT)
		goto only_push;

	uint16_t top_i = g_expression.stck_i - 1;
	Token_Type top = g_expression.values[top_i].asopt;

	while (exchange(is, top)) {
		push_at_beginning(NULL, top);
		g_expression.stck_i = top_i;

		if (top_i == STACK_STARTS_AT) break;
		top = g_expression.values[--top_i].asopt;
	}

	only_push:
	g_expression.values[g_expression.stck_i++] = value;
	return true;
}

static bool exchange (const Token_Type in, const Token_Type top)
{
	if (in == type_left_p) return false;
	static const uint16_t same_low = type_add * type_sub;
	static const uint16_t same_mid = type_mul * type_div * type_mod;

	uint16_t this = in * top;

	bool same = !(same_low % this) || !(same_mid % this);
	if (same)
		return true;
	return in < top;
}
