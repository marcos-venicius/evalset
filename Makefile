CXX = clang
CFLAGS = -Wall -Wextra -pedantic -ggdb

LEXER = lexer.c io.c
LEXER_OBJ = $(LEXER:.c=.o)

lexer: $(LEXER_OBJ)
	$(CXX) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf lexer *.o
