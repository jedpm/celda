#include "build.h"
#include "arith.h"
#define ER_NO_PARENT    0   /* Using { with no previous }................................ */
#define ER_NOL_SPACE    1   /* There is no longer space to save tokens/sub-expressions... */
#define ER_NO_EXPRSS    2   /* Empty expression or unknown operation to be solved........ */
#define ER_NO_IN_TBL    3   /* Reference outta bounds.................................... */
#define ER_SYNTAX_ERR   4   /* Unexpected value while solving expression................. */
#define MIN_OF(a, b)    ((a < b) ? a : b)

static void init_expression (Expr*, Expr*);
static void error_occurred (Cell*, uint8_t);
static bool check_space (Cell*, uint16_t, uint16_t);

static Token_Type solving_station (Spread*, Cell*, Expr*, char*);

static Token_Type solve_4_arith (Spread*, Cell*, Expr*, char*);
static Token_Type solve_4_conditionals (Spread*, Cell*, Expr*, char*);
static Token_Type get_content_of (Spread*, Cell*, Token*);
static bool check_4_same_type (Cell*, const Token_Type, const Token_Type);

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
    cc->type = type_unknown;
    cc->cex  = &cc->expression;
}

void build_init_row (Spread* sp)
{
    sp->cells[sp->cells_i - 1].first = true;
    sp->firsts[sp->first_i++] = sp->cells_i;
}

void build_save_token (Spread* sp, const char* token, size_t len, const Token_Type type)
{
    uint16_t cc_pos = sp->cells_i;
    if (!cc_pos--) {
        CELDA_WARNG("trying to save tokens with no previous cell");
        return;
    }

    Cell* cc = &sp->cells[cc_pos];
    /* If the first token of a cell is a constant value then
     * the cell is gonna be set to such content and it will
     * no longer accept any other token.
     * */
    if (CELDA_IS_CNST(cc->type)) {
        CELDA_WARNG("the %d cell was already set to '%s'", cc_pos, cc->cell);
        return;
    }

    Expr** ex = &cc->cex;
    if (CELDA_IS_CNST(type) && !cc->expression.token_i) {
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
    if (!CELDA_IS_CNST(cc->type))
        cc->type = solving_station(sp, cc, &cc->expression, cc->cell);

    //printf("(%d, %d) %s ", numcell++, cc->type, cc->cell);

    printf("%s |", cc->cell);
    if (cc->first) putchar(10);
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
        "!<NO_PARENT>",
        "!<NOL_SPACE>",
        "!<NO_EXPRES>",
        "!<NO_IN_TBL>",
        "!<SYNTAX_ER>"
    };

    const char* err = errors[kind];
    snprintf(cc->cell, strlen(err) + 1, "%s", err);
    cc->type = type_error;
}

static bool check_space (Cell* cc, uint16_t pos, uint16_t lim)
{
    if (pos < lim)
        return true;
    error_occurred(cc, ER_NOL_SPACE);
    return false;
}

void solve_token (Spread* sp, Cell* cc, Expr* ex, Token* t)
{
    if (CELDA_IS_CNST(t->type))
        return;

    if (t->type == type_reference)
        t->type = get_content_of(sp, cc, t);
    else
        t->type = solving_station(sp, cc, &ex->children[ex->child_i++], t->token);
}

static Token_Type solving_station (Spread* sp, Cell* cc, Expr* ex, char* put_in)
{
    if (!ex->token_i)
        goto no_expression;

    /* Is set to zero once again to know what is the next
     * children expressions to be solved when needed instead
     * of making another index. */
    ex->child_i = 0;

    switch (ex->tokens[0].type) {
        case type_arithmetic:
            return solve_4_arith(sp, cc, ex, put_in);

        case type_condition:
            return solve_4_conditionals(sp, cc, ex, put_in);
    }

    no_expression:
    error_occurred(cc, ER_NO_EXPRSS);
    return type_error;
}

static Token_Type solve_4_arith (Spread* sp, Cell* cc, Expr* ex, char* put_in)
{
    Arith a = arith_init();
    for (uint16_t i = 1; i < ex->token_i; i++) {
        Token* t = &ex->tokens[i];

        if (!CELDA_IS_MATH_SYMBOL(t->type))
            solve_token(sp, cc, ex, t);
        if (!t->type)
            return type_error;

        if (!arith_push(&a, t->token, t->type))
            goto syntax_err;
    }

    if (!arith_solve(&a, put_in))
        goto syntax_err;

    return type_number;

    syntax_err:
    error_occurred(cc, ER_SYNTAX_ERR);
    return type_error;
}

static Token_Type solve_4_conditionals (Spread* sp, Cell* cc, Expr* ex, char* put_in)
{
    if (ex->token_i < 6) {
        error_occurred(cc, ER_SYNTAX_ERR);
        return type_error;
    }

    // ? a x b c d
    // 0 1 2 3 4 5
    Token* a = &ex->tokens[1], *b = &ex->tokens[3];
    solve_token(sp, cc, ex, a);
    solve_token(sp, cc, ex, b);

    if (!a->type || !b->type)
        return type_error;

    if (a->type != b->type) {
        error_occurred(cc, ER_SYNTAX_ERR);
        return type_error;
    }

    snprintf(put_in, 7, "%s", "number");
}

static Token_Type get_content_of (Spread* sp, Cell* cc, Token* t)
{
    const char *on = t->token;
    const size_t nch = strcspn(on + 1, "1234567890");

    uint16_t row = atoi(on + 1 + nch), col = 0, pos;
    for (uint16_t i = 1; i <= nch; i++)
        col += tolower(on[i]) - 'a';

    pos = sp->firsts[row] + col;
    if ((row >= sp->first_i) || (pos >= sp->cells_i)) {
        error_occurred(cc, ER_NO_IN_TBL);
        return type_error;
    }

    Cell* such = &sp->cells[pos];
    snprintf(t->token, strlen(such->cell) + 1, "%s", such->cell);

    t->type = such->type;
    return t->type;
}

