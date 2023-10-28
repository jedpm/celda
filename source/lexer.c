#include "lexer.h"

static Token_Type resolve_type (const char, const char);
static void unknown_token_type (const char*, size_t*, size_t);
static size_t get_literal (const char*, size_t*, const Token_Type);

void lexer_lexer (char* content, size_t _len, uint16_t _rows, uint16_t _cells)
{
    Spread* sp = build_start(_rows, _cells);
    for (size_t i = 0; i < _len; i++) {
        const char a = content[i];

        /* When a vertical bar is found it means a new cell has begun
         * what implies that the prevuois cell is gonn be solved and the
         * new one gott get ready for accept tokens.
         * */
        if (a == '|') {
            build_solve_this(sp);
            build_init_cell(sp);
            continue;
        }
        if (a == '\n') { build_init_row(sp); continue; }
        if (isspace(a)) continue;

        const Token_Type type = resolve_type(a, content[i + 1]);
        if (type == type_unknown) {	
            unknown_token_type(content, &i, _len);
            continue;
        }

        if (CELDA_IS_LIT(type)) {
            size_t prev = i, len = get_literal(content, &i, type);
            build_save_token(sp, content + prev, len, type);
        }
        else {
            if (CELDA_IS_DOUBLE_FORMED(type)) i++;
            build_save_token(sp, "symbol", 6, type);
        }
    }

    free(content);
    build_solve_this(sp);
    puts("\n");
}

static Token_Type resolve_type (const char a, const char b)
{
    switch (a) {
        case '`': return type_string;
        case '&': return type_reference;
        case '$': return type_arithmetic;
        case '?': return type_condition;
        case '+': return type_add;
        case '-': return isdigit(b) ? type_number : type_sub;
        case '*': return type_mul;
        case '/': return type_div;
        case '%': return type_mod;
        case '^': return type_pow;
        case '(': return type_left_p;
        case ')': return type_rigth_p;
        case '{': return type_left_c;
        case '}': return type_rigth_c;
        case '>': return (b == '=') ? type_grequ  : type_greater;
        case '<': return (b == '=') ? type_leequ  : type_less;
        case '=': return (b == '=') ? type_equals : type_unknown;
        case '!': return (b == '=') ? type_nequal : type_unknown;
    }
    return isdigit(a) ? type_number : type_unknown;
}

/* This functions provies a context of what somthing found
 * on the table looks like, this is called when the current
 * character does not make sense to the lexer.
 * */
static void unknown_token_type (const char* context, size_t* _pos, size_t max)
{
    size_t pos = *_pos;
    uint16_t show = 0;

    char c = context[pos];
    while (!resolve_type(c, (pos + 1) < max ? context[pos + 1] : 0) && c >= 32 && pos < max) {
        c = context[++pos];
        show++;
    }

    CELDA_WARNG("unknown token <%.*s> at the %ld byte", show, context + *_pos, *_pos);
    *_pos = --pos;
}

/* Functions to get all literals defined on the table. */
static bool get_string (const char x) { return x  !=  '`'; }
static bool get_number (const char x) { return isdigit(x) || x == '.'; }
static bool get_referc (const char x) { return isalnum(x); }

static size_t get_literal (const char* context, size_t* _pos, const Token_Type kind)
{
    typedef bool (*fx) (const char);
    fx y = get_referc;

    /**/ if (kind == type_string) y = get_string;
    else if (kind == type_number) y = get_number;

    size_t len = 0;
    do {
        len++;
        *_pos += 1;
    } while (y(context[*_pos]));

    if (kind != type_string) *_pos -= 1;
    else len++;

    return len;
}

