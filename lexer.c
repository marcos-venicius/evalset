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

void lexer_free(Lexer *lexer) {
    free(lexer->content);
}

int main() {
    Lexer lexer = create_lexer("./examples/settings.es");

    printf("Lexing:\n%s\n", lexer.content);

    lexer_free(&lexer);

    return 0;
}
