CXX = clang
CFLAGS = -Wall -Wextra -pedantic -ggdb

evalset: evalset.o parser.o lexer.o io.o utils.o print.o
	$(CXX) $(CFLAGS) -o evalset $^

parser.o: parser.h parser.c loc.h lexer.h
	$(CXX) $(CFLAGS) -c parser.c -o parser.o

lexer.o: lexer.c lexer.h utils.h
	$(CXX) $(CFLAGS) -c lexer.c -o lexer.o

utils.o: utils.c utils.h
	$(CXX) $(CFLAGS) -c utils.c -o utils.o

print.o: print.c print.h parser.h
	$(CXX) $(CFLAGS) -c print.c -o print.o

io.o: io.c io.h
	$(CXX) $(CFLAGS) -c io.c -o io.o

evalset.o: evalset.c io.h parser.h lexer.h print.h
	$(CXX) $(CFLAGS) -c evalset.c -o evalset.o

clean:
	rm -rf evalset *.o
