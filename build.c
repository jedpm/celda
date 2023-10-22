#include "build.h"
#include "arith.h"
#define ER_NO_PARENT    0
#define ER_NOL_SPACE    1
#define ER_NO_TOKENS    2
#define ER_NO_EXPRSS    3
#define ER_NO_IN_TBL    4
#define ER_UNEXPECTED   5
#define ER_SYNTAX_ERR   6
#define min(a, b) ((a < b) ? a : b)

static void init_expression (Expr*, Expr*);
static void error_occurred (Cell*, uint8_t);
static bool check_space (Cell*, uint16_t, uint16_t);

static Token_Type solving_station (Spread*, Cell*, Expr*, char*);
static void solve_4_arith (Spread*, Cell*, Expr*, char*);
static Token_Type solve_4_conditionals (Spread*, Cell*, Expr*, char*);

static bool get_content_of (Spread*, Cell*, Token*, const Token_Type);
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

    Cell* cc = &sp->cells[cc_pos]; // XXX: Maybe have a current cell would be a good idea
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
    if (CELDA_IS_CNST(cc->type)) goto print;

    if (cc->expression.token_i)
        solving_station(sp, cc, &cc->expression, cc->cell);
    else
        cc->type = type_error;

    print:
    if (cc->first) putchar(10);
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
        "!<NO_PARENT>",
        "!<NOL_SPACE>",
        "!<NO_TOKENS>",
        "!<NO_EXPRES>",
        "!<NO_IN_TBL>",
        "!<~EXPECTED>",
        "!<SYNTAX_ER>"
    };

    const char* err = errors[kind];
    snprintf(cc->cell, strlen(err) + 1, "%s", err);
    cc->type = (kind == ER_NO_TOKENS) ? type_unknown : type_error;
}

static bool check_space (Cell* cc, uint16_t pos, uint16_t lim)
{
    if (pos < lim)
        return true;
    error_occurred(cc, ER_NOL_SPACE);
    return false;
}

static Token_Type solving_station (Spread* sp, Cell* cc, Expr* ex, char* put_in)
{
    if (!ex->token_i) {
        error_occurred(cc, ER_NO_TOKENS);
        return type_error;
    }

    switch (ex->tokens[0].type) {
        case type_arithmetic:
            solve_4_arith(sp, cc, ex, put_in);
            return type_number;

        case type_condition:
            solve_4_conditionals(sp, cc, ex, put_in);
            return type_unknown;

        default:
            error_occurred(cc, ER_NO_EXPRSS);
            return type_error;
    }
}


// XXX: May be it is a good idea got a pointer to the type to modify it
static void solve_4_arith (Spread* sp, Cell* cc, Expr* ex, char* put_in)
{
    Arith art = arith_init();
    uint16_t csub_ex = 0;

    for (uint16_t i = 1; i < ex->token_i; i++) {
        Token* t = &ex->tokens[i];

        /* Left brace token means there is a sub-expression,
         * then it must be solved first before continue with
         * the current one. */
        if (t->type == type_left_c) {
            const Token_Type did = solving_station(sp, cc, &ex->children[csub_ex++], t->token);
            if (!CELDA_IS_NUMBER(did) || cc->type == type_error)
                return;
            t->type = did;
        }

        else if (t->type == type_reference && !get_content_of(sp, cc, t, type_number))
            return;
        else if (t->type != type_number && !CELDA_IS_MATH_SYMBOL(t->type))
            goto error;
        if (!arith_push(&art, t->token, t->type))
            goto error;
    }

    if (!arith_solve(&art, put_in))
        goto error;

    cc->type = type_number;
    return;
    error: error_occurred(cc, ER_SYNTAX_ERR);
}


static Token_Type solve_4_conditionals (Spread* sp, Cell* cc, Expr* ex, char* put_in)
{
    /* The minimum of tokens is 6 since a conditional is:
     * <?> <value> <condition> <value> <if_it_is> <if_it_aint>
     * */
    if (ex->token_i < 6 || !CELDA_CONDITION_SYMBOL(ex->tokens[2].type)) {
        error_occurred(cc, ER_SYNTAX_ERR);
        return type_error;
    }
    uint16_t csub_ex = 0;

    Token* a = &ex->tokens[1], *b = &ex->tokens[3];
    if (a->type == type_left_c)
        a->type = solving_station(sp, cc, &ex->children[csub_ex++], a->token);
    if (b->type == type_left_c)
        b->type = solving_station(sp, cc, &ex->children[csub_ex++], b->token);

    if (a->type != b->type) {
        error_occurred(cc, ER_UNEXPECTED);
        return type_error;
    }

    int its = memcmp(a->token, b->token, min(strlen(a->token), strlen(b->token)));

    Token* ans = NULL;
    switch (ex->tokens[2].type) {
        case type_equals:
            ans = (!its) ? &ex->tokens[4] : &ex->tokens[5];
            break;
        case type_nequal:
            ans = (its) ? &ex->tokens[4] : &ex->tokens[5];
            break;
        case type_greater:
            ans = (its > 0) ? &ex->tokens[4] :  &ex->tokens[5];
            break;
        case type_grequ:
            ans = (its > -1) ? &ex->tokens[4] :  &ex->tokens[5];
            break;
        case type_less:
            ans = (its < 0) ? &ex->tokens[4] :  &ex->tokens[5];
            break;
        case type_leequ:
            ans = (its < 1) ? &ex->tokens[4] :  &ex->tokens[5];
            break;
        default:
            // XXX: the error can also be set in here.
            break;
    }

    if (!ans)
        puts("oops");

    if (ans->type == type_left_c)
        ans->type = solving_station(sp, cc, &ex->children[csub_ex++], ans->token);

    snprintf(put_in, strlen(ans->token) + 1, "%s", ans->token);
    return ans->type;
}

static bool get_content_of (Spread* sp, Cell* cc, Token* t, const Token_Type must_b)
{
    const char* addr = t->token;
    const size_t nch = strcspn(addr + 1, "1234567890");

    uint16_t row = atoi(addr + 1 + nch), col = 0;
    for (uint16_t i = 1; i <= nch; i++)
        col += tolower(addr[i]) - 'a';

    uint16_t pos = sp->firsts[row] + col;
    if ((row >= sp->first_i) || (pos >= sp->cells_i)) {
        error_occurred(cc, ER_NO_IN_TBL);
        return false;
    }

    Cell* ths = &sp->cells[pos];
    if (!check_4_same_type(cc, ths->type, must_b))
        return false;

    snprintf(t->token, strlen(ths->cell) + 1, "%s", ths->cell);
    t->type = must_b;
    return true;
}

static bool check_4_same_type (Cell* cc, const Token_Type a, const Token_Type b)
{
    if (a == b)
        return true;
    error_occurred(cc, ER_UNEXPECTED);
    return false;
}

