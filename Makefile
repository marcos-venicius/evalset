CXX = clang
CFLAGS = -Wall -Wextra -pedantic -ggdb

evalset: evalset.o parser.o lexer.o io.o
	$(CXX) $(CFLAGS) -o evalset $^

parser.o: parser.h parser.c loc.h lexer.h
	$(CXX) $(CFLAGS) -c parser.c -o parser.o

lexer.o: lexer.c lexer.h
	$(CXX) $(CFLAGS) -c lexer.c -o lexer.o

io.o: io.c io.h
	$(CXX) $(CFLAGS) -c io.c -o io.o

evalset.o: evalset.c io.h lexer.h
	$(CXX) $(CFLAGS) -c evalset.c -o evalset.o

clean:
	rm -rf evalset *.o
