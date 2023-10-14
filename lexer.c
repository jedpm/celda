#include "lexer.h"

static Token_Type resolve_type (const char, const char);
static void unknown_token_type (const char*, size_t);
static size_t get_literal (const char*, size_t*, const Token_Type);

void lexer_lexer (char* content, size_t _len, uint16_t _rows, uint16_t _cells)
{
	for (size_t i = 0; i < _len; i++) {
		const char a = content[i];

		if (a == '|') continue;
		if (a == '\n') continue;
		if (isspace(a)) continue;

		const Token_Type type = resolve_type(a, content[i + 1]);
		if (type == type_unknown) {	
			unknown_token_type(content, i);
			continue;
		}

		if (CELDA_IS_LIT(type)) {
			size_t prev = i, len = get_literal(content, &i, type);
			printf("token: %.*s\n", (int) len, content + prev);
		}
	}

	free(content);
}

static Token_Type resolve_type (const char a, const char b)
{
	switch (a) {
		case '`': return type_string;
		case '&': return type_reference;
	}

	return isdigit(a) ? type_number : type_unknown;
}

static void unknown_token_type (const char* context, size_t _pos)
{
	size_t pos = _pos;
	uint16_t show = 0;

	do {
		show++;
		pos++;	
	} while (context[pos] >= 32 && show <= 10);

	CELDA_WARNG("unknown token type: '%.*s...' at %ld byte", show, context + _pos, _pos);
}

static bool get_string (const char x) { return x != '`'; }
static bool get_number (const char x) { return isdigit(x); }
static bool get_referc (const char x) { return isdigit(x) || isupper(x) || islower(x); }

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

