#include "build.h"
#include "arith.h"
#define ER_NO_TOKENS    0
#define ER_NO_PARENT    1
#define ER_NO_SPACE     2

static void init_expression (Expr*, Expr*);
static void error_occurred (Cell*, uint8_t);
static bool check_space (Cell*, uint16_t, uint16_t);

Spread* build_start (uint16_t rows, uint16_t cells)
{
    Spread* spread = (Spread*) calloc(1, sizeof(Spread));
    CELDA_CHECK_MEM(spread);

    spread->cells = (Cell*) calloc(cells, sizeof(Cell));
    CELDA_CHECK_MEM(spread->cells);

    spread->firsts = (uint16_t*) calloc(rows, sizeof(uint16_t));
    CELDA_CHECK_MEM(spread->firsts);

    spread->cells_i  = 0;
    spread->first_i  = 1;

    return spread;
}

void build_init_cell (Spread* sp)
{
    Cell* cc = &sp->cells[sp->cells_i++];
    init_expression(&cc->expression, NULL);
    error_occurred(cc, ER_NO_TOKENS);

    cc->cex = &cc->expression;
}

void build_init_row (Spread* sp)
{
    if (!sp->cells_i)
        return;
    sp->cells[sp->cells_i - 1].first = true;
    sp->firsts[sp->first_i++] = sp->cells_i;
}

void build_save_token (Spread* sp, const char* token, size_t len, const Token_Type type)
{
    static int a = 0;
    uint16_t cc_pos = sp->cells_i;

    if (!cc_pos--) {
		CELDA_WARNG("tryna save tokens without previous cell");
		return;
    }

	Cell* cc = &sp->cells[cc_pos];
	if (CELDA_IS_CNST(cc->type)) {
		CELDA_WARNG("the %d cell was already set to '%s'", cc_pos, cc->cell);
		return;
	}


    Expr** ex = &cc->cex;
	if (CELDA_IS_CNST(type) && !(*ex)->token_i) {
		snprintf(cc->cell, len + 1, "%.*s", (int) len, token);
		cc->type = type;
		return;
	}

	if (type == type_rigth_c) {
		if (!(*ex)->parent) error_occurred(cc, ER_NO_PARENT);
		else *ex = (*ex)->parent;
		return;
	}

	if (!check_space(cc, (*ex)->token_i, CELDA_TOKEN_PER_EXP))
		return;

	Token this = { .type = type };
	memcpy(this.token, token, len);
	(*ex)->tokens[(*ex)->token_i++] = this;

	if (type == type_left_c) {
		if (!check_space(cc, (*ex)->child_i, CELDA_SUB_EXP_PER_EXP)) return;

        Expr* new = &(*ex)->children[(*ex)->child_i++];
        init_expression(new, *ex);
        *ex = new;
	}
}

static void init_expression (Expr* ex, Expr* parent)
{
    ex->children = (Expr*) calloc(CELDA_SUB_EXP_PER_EXP, sizeof(Expr));
    CELDA_CHECK_MEM(ex->children);

    ex->parent  = parent;
    ex->child_i = 0;
    ex->token_i = 0;
}

static void error_occurred (Cell* cc, uint8_t kind)
{
	static const char* errors[] = {
		"!<IS_EMPTY>",
        "!<NO_PARENT>",
        "!<NO_SPACE>"
	};

	const char* err = errors[kind];
	snprintf(cc->cell, strlen(err) + 1, "%s", err);
	cc->type = (kind == ER_NO_TOKENS) ? type_unknown : type_error;
}

static bool check_space (Cell* cc, uint16_t pos, uint16_t lim)
{
	if (pos != lim)
		return true;
	error_occurred(cc, ER_NO_SPACE);
	return false;
}


/*static Spread g_spread  = {0};
static Expr* g_cur_expr = NULL;

static void init_expression (Expr*);
static void set_cell_to_err (Cell*, uint8_t);
static bool check_space (Cell*, uint16_t, uint16_t);
static void solve_cell (Cell*);
static void solve_arithmetic (Cell*, Expr*);
static bool value_of (Cell*, Token*, const Token_Type);

void build_start (uint16_t rows, uint16_t cells)
{
	g_spread.cells = (Cell*) calloc(cells, sizeof(Cell));
	CELDA_CHECK_MEM(g_spread.cells);

	g_spread.firsts = (uint16_t*) calloc(rows, sizeof(uint16_t));
	CELDA_CHECK_MEM(g_spread.firsts);

	g_spread.cells_i = 0;
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
		"!<MALFORMED>",
		"!<OUTTABLE>",
		"!<UNEXPECTED_VALUE>"
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
	// $3 / {$2 + 5}
	Arith expression = arith_init();

	for (uint16_t i = 1; i < ex->token_i; i++) {	
		Token* t = &ex->tokens[i];

		if (t->type == type_reference && !value_of(cc, t, type_number))
			return;

		else if (t->type != type_number && !CELDA_IS_MATH_SYMBOL(t->type))
			goto malformed;

		if (!(arith_push(&expression, t->token, t->type)))
			goto malformed;
	}

	if (!arith_solve(&expression, cc->cell))
		goto malformed;

	cc->type = type_number;
	return;

	malformed:
	set_cell_to_err(cc, ERROR_MALFORMED);
	return;
}

static bool value_of (Cell* cc, Token* tk, const Token_Type expected)
{
	char* addr = tk->token;
	const size_t len = strlen(addr);

	char rowas[10];
	uint16_t col = 0, row = 0;

	for (size_t i = 1; i < len; i++) {
		const char a = addr[i];
		if (islower(a) || isupper(a))
			col += tolower(a) - 'a';
		if (isdigit(a))
			rowas[row++] = a - '0' + '0';
	}

	row = atoi(rowas);
	if ((row >= g_spread.first_i) || (g_spread.firsts[row] + col >= g_spread.cells_i)) {
		set_cell_to_err(cc, ERROR_OUTTABLE);
		return false;
	}

	uint16_t pos = g_spread.firsts[row] + col;
	Cell* c = &g_spread.cells[pos];

	if (c->type != expected) {
		set_cell_to_err(cc, ERROR_UNEXPECTED);
		return false;
	}

	snprintf(tk->token, 1 + strlen(c->cell), "%s", c->cell);
	tk->type = expected;
	return true;
}
*/
