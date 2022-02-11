CC = g++

CFLAGS = -c

OBJ_FILES = main.o Lexer.o Parser.o Differentiator.o ./Stack/Stack.o Reading_File.o Log_File.o

FUNC_FILE = Function.txt

Differentiator: main.o Lexer.o Parser.o Differentiator.o Stack.o Reading_File.o Log_File.o
	$(CC) $(OBJ_FILES) -o Differentiator.out

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp -o main.o

Lexer.o: Lexer.cpp
	$(CC) $(CFLAGS) Lexer.cpp -o Lexer.o

Parser.o: Parser.cpp
	$(CC) $(CFLAGS) Parser.cpp -o Parser.o

Differentiator.o: Differentiator.cpp
	$(CC) $(CFLAGS) Differentiator.cpp -o Differentiator.o

Stack.o: ./Stack/Stack.cpp
	$(CC) $(CFLAGS) ./Stack/Stack.cpp -o ./Stack/Stack.o

Reading_File.o: Reading_File.cpp
	$(CC) $(CFLAGS) Reading_File.cpp -o Reading_File.o

Log_File.o: Log_File.cpp
	$(CC) $(CFLAGS) Log_File.cpp -o Log_File.o

run:
	./Differentiator.out $(FUNC_FILE)

clean:
	rm $(OBJ_FILES)
	rm -rf *.dot
	rm Differentiator.out