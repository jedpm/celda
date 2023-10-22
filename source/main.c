#include "lexer.h"
#include "arith.h"

static char* contents (FILE*, size_t*);
static void info_about (const char*, uint16_t*, uint16_t*);

int main (int argc, char** argv)
{
    if (argc != 2)
        CELDA_ERROR("Pass the table to be parsed as unique argument");

    FILE* file = fopen(argv[1], "r");
    if (!file)
        CELDA_ERROR("file given does not work");

    size_t content_len;
    char* content = contents(file, &content_len);

    uint16_t rows = 0, cells = 0;
    info_about(content, &rows, &cells);

    lexer_lexer(content, content_len, rows, cells);
    return 0;
}

static char* contents (FILE* file, size_t* bytes)
{
    fseek(file, 0, SEEK_END);
    *bytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (!*bytes) {
        CELDA_WARNG("file provied got no content");
        exit(EXIT_SUCCESS);
    }

    char* content = (char*) calloc(*bytes + 1, 1);
    CELDA_CHECK_MEM(content);

    fread(content, *bytes, 1, file);
    content[*bytes] = 0;

    fclose(file);
    return content;
}

/* TODO: Use an .sh program instead. */
static void info_about (const char* content, uint16_t* rows, uint16_t* cells)
{
    size_t a = 0;
    while (content[a]) {
        const char b = content[a++];
        /**/ if (b == '\n') *rows += 1;
        else if (b == '|')  *cells += 1;
    }
}