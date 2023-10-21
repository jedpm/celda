#include "build.h"
#include "arith.h"
#define ER_NO_TOKENS    0
#define ER_NO_PARENT    1
#define ER_NO_SPACE     2
#define ER_SYNTAX       3
#define ER_OUTTA_BOUDNS 4
#define ER_UNEXPECTED   5

static void init_expression (Expr*, Expr*);
static void error_occurred (Cell*, uint8_t);
static bool check_space (Cell*, uint16_t, uint16_t);
static void findout_its_op (Spread*, Cell*, char*, const Token_Type);
static void it_is_for_maths (Spread*, Cell*, char*);
static bool get_value_from (Spread*, Cell*, Token*, const Token_Type);

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

void build_solve_this (Spread* sp)
{
    static uint16_t numcell = 0;
    if (!sp->cells_i)
        return;

    Cell* cc = &sp->cells[sp->cells_i - 1];
    if (cc->first) putchar(10);

    if (cc->expression.token_i)
        findout_its_op(sp, cc, cc->cell, cc->expression.tokens[0].type);
    printf("(%d, %d) %s ", numcell++, cc->type, cc->cell);
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
        "!<NO_SPACE>",
        "!<ER_SYNTAX>",
        "!<BOUNDS>",
        "!<UNEXPECTED>"
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

static void findout_its_op (Spread* sp, Cell* cc, char* setin, const Token_Type op)
{
    if (CELDA_IS_CNST(op) || !cc->expression.token_i)
        return;

    switch (op) {
        case type_arithmetic: it_is_for_maths(sp, cc, setin); break;
    }
}


static void it_is_for_maths (Spread* sp, Cell* cc, char* setin)
{
    Arith art = arith_init();
    Expr* ex  = &cc->expression;

    for (uint16_t i = 1; i < ex->token_i; i++) {
        Token* t = &ex->tokens[i];

        /**/ if (t->type == type_reference && !get_value_from(sp, cc, t, type_number))
            return;
        else if (t->type != type_number && !CELDA_IS_MATH_SYMBOL(t->type))
            goto syntax_err;
        if (!arith_push(&art, t->token, t->type))
            goto syntax_err;
    }

    if (!arith_solve(&art, setin))
        goto syntax_err;

    cc->type = type_number;
    return;
    syntax_err: error_occurred(cc, ER_SYNTAX);
}

static bool get_value_from (Spread* sp, Cell* cc, Token* t, const Token_Type must_b)
{
    const char* addr = t->token;
    const size_t len = strlen(addr);

    /* there must be a better way but je ne
     * sais pas como. */
    char row_[10] = {0};

    uint16_t row = 0, col = 0;
    for (size_t i = 1; i < len; i++) {
        const char a = addr[i];
        if (isdigit(a)) row_[row++] = a - '0' + '0';
        else col += tolower(a) - 'a';
    }

    row = atoi(row_);
    uint16_t pos = sp->firsts[row] + col;

    if ((row >= sp->first_i) || (pos >= sp->cells_i)) {
        error_occurred(cc, ER_OUTTA_BOUDNS);
        return false;
    }


    Cell* that;
    if ((that = &sp->cells[pos])->type != must_b) {
        error_occurred(cc, ER_UNEXPECTED);
        return false;
    }

    snprintf(t->token, strlen(that->cell) + 1, "%s", that->cell);
    t->type = that->type;
    return true;
}

