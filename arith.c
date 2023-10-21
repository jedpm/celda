#include "arith.h"
#include <math.h>

static Arith g_expression;

static bool push_at_beginning (Arith*, const char*, const Token_Type);
static bool push_at_stack (Arith*, const Token_Type);
static bool need_2_exchange (const Token_Type, const Token_Type);
static bool end_of_parentheses (Arith*);
static void solve (double*, double, const Token_Type);

Arith arith_init ()
{
	Arith arth = {
	.stck_i = STACK_STARTS_AT
	};
	return arth;
}

bool arith_push (Arith* self, const char* as, const Token_Type is)
{
	if (is == type_number)
		return push_at_beginning(self, as, is);
	return push_at_stack(self, is);
}

bool arith_solve (Arith* self, char* ans)
{
	uint16_t i;
	for (i = self->stck_i - 1; i >= STACK_STARTS_AT; i--)
		push_at_beginning(self, NULL, self->values[i].asopt);

	double numstack[3] = {0};
	uint16_t nums_i = 0;

	for (i = 0; i < self->expr_i; i++) {
		Val *v = &self->values[i];
		if (v->asopt == type_number)
			numstack[nums_i++] = v->asnum;
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

static bool push_at_beginning (Arith* self, const char* as, const Token_Type is)
{
	if (self->expr_i == STACK_STARTS_AT)
		return false;

	Val value = {	
		.asnum = (as) ? atol(as) : 0,
		.asopt = is
	};

	self->values[self->expr_i++] = value;
	return true;
}

static bool push_at_stack (Arith* self, const Token_Type is)
{
	if (self->stck_i == EXPRESSION_SIZE)
		return false;

	if (is == type_rigth_p)
		return end_of_parentheses(self);

	if (is == type_left_p)
		self->parentheses++;

	Val value = { .asopt = is };
	if (self->stck_i == STACK_STARTS_AT)
		goto only_push;

	uint16_t top_i = self->stck_i - 1;
	Token_Type top = self->values[top_i].asopt;

	while (need_2_exchange(is, top)) {
		push_at_beginning(self, NULL, top);
		self->stck_i = top_i;

		if (top_i == STACK_STARTS_AT) break;
		top = self->values[--top_i].asopt;
	}

	only_push:
	self->values[self->stck_i++] = value;
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

static bool end_of_parentheses (Arith* self)
{
	if (!self->parentheses)
		return false;

	uint16_t pos = --self->stck_i;
	while (self->values[pos].asopt != type_left_p)
		push_at_beginning(self, NULL, self->values[pos--].asopt);

	self->stck_i = pos;
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

