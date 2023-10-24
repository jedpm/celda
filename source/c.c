Token_Type solve_token (Spread* sp, Cell* cc, Expr* ex, Token* t, const Token_Type expected)
{
    if (CELDA_IS_CNST(t->type))
        return t->type;
    if (t->type == type_reference && !get_content_of(sp, cc, t, expected))
        return type_error;
    else
        return solving_station(sp, cc, e, ex, t->token);
}
