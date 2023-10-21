#include "build.h"
#include "arith.h"
#define ER_NO_TOKENS    0
#define ER_NO_PARENT    1
#define ER_NO_SPACE     2

static void init_expression (Expr*, Expr*);
static void error_occurred (Cell*, uint8_t);
static bool check_space (Cell*, uint16_t, uint16_t);
static void findout_its_op (Cell*);

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
        return NULL;

    Cell* cc = &sp->cells[sp->cells_i - 1];
    if (cc->first)
        putchar(10);

    findout_its_op(cc);
    printf("(%d) %s ", numcell++, cc->cell);
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

static void findout_its_op (Cell* cc)
{
    if (CELDA_IS_CNST(cc->type) || !cc->expression.token_i)
        return;

    switch (cc->expression.tokens[0].type) {
        case type_arithmetic: puts("math");
    }
}


