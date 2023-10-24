void solve_token (Spread* sp, Cell* cc, Expr* ex, Token* t)
{
    if (CELDA_IS_CNST(t->type))
        return;

    if (t->type == type_reference && !get_content_of(sp, cc, t))
        t->type = type_error;
    else
        t->type = solving_station(sp, cc, &ex->children[ex->child_i++], t->token);
}
