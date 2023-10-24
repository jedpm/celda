#include "lexer.h"

static char* contents (FILE*, size_t*);

int main (int argc, char** argv)
{
    if (argc != 4) goto usage;
    FILE* file = fopen(argv[1], "r");

    if (!file) goto usage;

    size_t content_len;
    char* content = contents(file, &content_len);

    lexer_lexer(content, content_len, atoi(argv[2]), atoi(argv[3]));
    return 0;

    usage:
    CELDA_ERROR("use the .sh file to execute the program instead of this one");
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
