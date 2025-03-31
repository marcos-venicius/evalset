#include "./lexer.h"
#include "./parser.h"
#include "./io.h"

int main(void) {
    const char *filename = "./examples/settings.es";

    char *content;

    size_t data_size = read_from_file(filename, &content);

    Lexer lexer = create_lexer(filename, content, data_size);

    Token *head;

    if ((head = lex(&lexer))) {
        print_tokens(head);
    }

    Parser parser = parse_tokens(head);

    parser_free(parser);
    lexer_free(&lexer);

    return 0;
}
