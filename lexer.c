#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned long content_size;

    unsigned int cursor, bot, line, col;

    char *content;
    const char *filename;
} Lexer;

Lexer create_lexer(const char *filename) {
    FILE *fptr = fopen(filename, "r");

    if (fptr == NULL) {
        fprintf(stderr, "could not open file %s due to: %s\n", filename, strerror(errno));
        exit(1);
    }

    fseek(fptr, 0, SEEK_END);
    const size_t stream_size = ftell(fptr);
    rewind(fptr);
    
    Lexer lexer = (Lexer){
        .filename = filename,
        .col = 1,
        .line = 1,
        .cursor = 0,
        .bot = 0,
        .content_size = stream_size
    };

    lexer.content = malloc((stream_size + 1) * sizeof(char));

    const size_t read_size = fread(lexer.content, 1, stream_size, fptr);

    if (read_size != stream_size) {
        fprintf(stderr, "could not read file %s due to: %s\n", filename, strerror(errno));
        fclose(fptr);
        exit(1);
    }

    lexer.content[stream_size] = '\0';

    fclose(fptr);

    return lexer;
}

// Get the current char without moving the cursor
char chr(Lexer *lexer) {
    if (lexer->cursor < lexer->content_size) {
        return lexer->content[lexer->cursor];
    }

    return '\0';
}

// Advances the cursor to the next char
char nchr(Lexer *lexer) {
    if (lexer->cursor + 1 < lexer->content_size) {
        ++lexer->cursor;

        return chr(lexer);
    }

    return '\0';
}

// Lookahead of 1 char
char pchr(Lexer *lexer) {
    if (lexer->cursor + 1 < lexer->content_size) {
        return lexer->content[lexer->cursor + 1];
    }

    return '\0';
}

void lexer_free(Lexer *lexer) {
    free(lexer->content);
}

int main() {
    Lexer lexer = create_lexer("./examples/settings.es");

    printf("Lexing:\n%s\n", lexer.content);

    lexer_free(&lexer);

    return 0;
}
