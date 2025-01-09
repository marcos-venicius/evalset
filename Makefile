CXX = clang
CFLAGS = -Wall -Wextra -pedantic -ggdb

lexer: lexer.c
	$(CXX) $(CFLAGS) -o $@ $<

clean:
	rm -rf lexer
