#include "build.h"
#include "arith.h"
#define ERROR_EMPTINESS		0
#define ERROR_BOUNDS_BROKEN	1
#define ERROR_UNKNOWN_OPERATION	2
#define ERROR_MALFORMED		3

static Spread g_spread  = {0};
static Expr* g_cur_expr = NULL;

static void init_expression (Expr*);
static void set_cell_to_err (Cell*, uint8_t);
static bool check_space (Cell*, uint16_t, uint16_t);
static void solve_cell (Cell*);
static void solve_arithmetic (Cell*, Expr*);

void build_start (uint16_t rows, uint16_t cells)
{
	g_spread.cells = (Cell*) calloc(cells, sizeof(Cell));
	CELDA_CHECK_MEM(g_spread.cells);

	g_spread.firsts = (uint16_t*) calloc(rows, sizeof(uint16_t));
	CELDA_CHECK_MEM(g_spread.firsts);

	g_spread.cells_i = 0;
	/*
	 * 'firsts' saves the position on the table where a new row
	 * has been created, since the first position is zero and
	 * calloc starts all to zero then there is no needed to set
	 * it again.
	 * */
	g_spread.first_i = 1;
	g_spread.is_first = false;
}

void build_row ()
{
	g_spread.firsts[g_spread.first_i++] = g_spread.cells_i;
	g_spread.is_first = true;
}

void build_cell ()
{
	g_cur_expr = NULL;

	Cell* cc = &g_spread.cells[g_spread.cells_i++];
	init_expression(&cc->expression);
	set_cell_to_err(cc, ERROR_EMPTINESS);
	cc->first = g_spread.is_first;

	g_spread.is_first = false;
}

void build_token (const char* token, size_t len, const Token_Type type)
{
	uint16_t cellsmade = g_spread.cells_i;
	if (!cellsmade) {
		CELDA_WARNG("tryna save tokens without previous cell");
		return;
	}

	Cell* cc = &g_spread.cells[--cellsmade];
	if (CELDA_IS_CNST(cc->type)) {
		CELDA_WARNG("the %d cell was already set to '%s'", cellsmade, cc->cell);
		return;
	}

	Expr* ex = &cc->expression;
	if (CELDA_IS_CNST(type) && !ex->token_i) {
		snprintf(cc->cell, len + 1, "%.*s", (int) len, token);
		cc->type = type;

		return;
	}

	if (type == type_rigth_c) {
		if (!ex->parent) CELDA_WARNG("there is not parent to go to");
		else g_cur_expr = g_cur_expr->parent;
		return;
	}

	if (!check_space(cc, ex->token_i, CELDA_TOKEN_PER_EXP))
		return;

	Token this = { .type = type };
	memcpy(this.token, token, len);
	ex->tokens[ex->token_i++] = this;

	if (type == type_left_c) {
		if (!check_space(cc, ex->child_i, CELDA_SUB_EXP_PER_EXP)) return;
		init_expression(&ex->children[ex->child_i++]);
	}
}

void build_build ()
{
	putchar(10);
	putchar(10);

	for (uint16_t i = 0; i < g_spread.cells_i; i++) {
		Cell* cc = &g_spread.cells[i];
		if (cc->first) putchar(10);
		solve_cell(cc);
		printf("(%d) %s\t", i, cc->cell);
	}

	putchar(10);
	putchar(10);
}

static void init_expression (Expr* self)
{
	self->children = (Expr*) calloc(CELDA_SUB_EXP_PER_EXP, sizeof(Expr));
	CELDA_CHECK_MEM(self->children);

	self->parent  = g_cur_expr;
	self->token_i = 0;
	self->child_i = 0;

	g_cur_expr = self;
}

static void set_cell_to_err (Cell* cc, uint8_t to)
{
	static const char* errors[] = {
		"!<EMPTY>",
		"!<BOUNDS>",
		"!<UNKNOWN_OP>",
		"!<MALFORMED>"
	};

	const char* err = errors[to];
	snprintf(cc->cell, strlen(err) + 1, "%s", err);
	cc->type = (to == ERROR_EMPTINESS) ? type_unknown : type_error;
}

static bool check_space (Cell* cc, uint16_t pos, uint16_t lim)
{
	if (pos != lim)
		return true;

	set_cell_to_err(cc, ERROR_BOUNDS_BROKEN);
	return false;
}


static void solve_cell (Cell* cc)
{
	Expr* ex = &cc->expression;
	if (!ex->token_i)
		return;

	switch (ex->tokens[0].type) {	
		case type_arithmetic: {
			solve_arithmetic(cc, ex);
			break;
		}
		default: {
			set_cell_to_err(cc, ERROR_UNKNOWN_OPERATION);
			break;
		}
	}
}

static void solve_arithmetic (Cell* cc, Expr* ex)
{
	arith_init();
	bool success = true;

	for (uint16_t i = 1; i < ex->token_i; i++) {	
		Token* t = &ex->tokens[i];
		const Token_Type tp = t->type;

		if (tp == type_number)
			success = arith_push(t->token, t->type);
		else if (CELDA_IS_MATH_SYMBOL(tp))
			success = arith_push(NULL, t->type);
		else if (tp == type_reference) {
			puts("no yet");
			exit(0);
		}
		else
			goto malformed;

		if (!success)
			goto malformed;
	}

	if (!arith_solve(cc->cell))
		goto malformed;
	cc->type = type_number;
	return;

	malformed:
	set_cell_to_err(cc, ERROR_MALFORMED);
	return;		
}
