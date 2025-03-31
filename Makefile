CXX = clang
CFLAGS = -Wall -Wextra -pedantic -ggdb

evalset: parser.o lexer.o io.o
	$(CXX) $(CFLAGS) -o evalset $^

parser.o: parser.h
	$(CXX) $(CFLAGS) -c parser.c -o parser.o

lexer.o: lexer.c lexer.h io.c io.h
	$(CXX) $(CFLAGS) -c lexer.c -o lexer.o

io.o: io.c io.h
	$(CXX) $(CFLAGS) -c io.c -o io.o

clean:
	rm -rf evalset *.o
