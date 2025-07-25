CXX = clang
CFLAGS = -Wall -Wextra -pedantic -ggdb
EXE_NAME = evalset

$(EXE_NAME): evalset.o parser.o lexer.o io.o utils.o print.o interpreter.o map.o
	$(CXX) $(CFLAGS) -o $(EXE_NAME) $^

parser.o: parser.h parser.c loc.h lexer.h
	$(CXX) $(CFLAGS) -c parser.c -o parser.o

lexer.o: lexer.c lexer.h utils.h loc.h
	$(CXX) $(CFLAGS) -c lexer.c -o lexer.o

utils.o: utils.c utils.h
	$(CXX) $(CFLAGS) -c utils.c -o utils.o

print.o: print.c print.h parser.h
	$(CXX) $(CFLAGS) -c print.c -o print.o

io.o: io.c io.h
	$(CXX) $(CFLAGS) -c io.c -o io.o

map.o: map.c map.h utils.h
	$(CXX) $(CFLAGS) -c map.c -o map.o

interpreter.o: interpreter.c interpreter.h parser.h map.h loc.h utils.h assertf.h
	$(CXX) $(CFLAGS) -c interpreter.c -o interpreter.o

evalset.o: evalset.c io.h parser.h lexer.h print.h
	$(CXX) $(CFLAGS) -c evalset.c -o evalset.o

clean:
	rm -rf $(EXE_NAME) *.o
