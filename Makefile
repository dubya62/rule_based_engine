
CC :=gcc
CFLAGS :=-O3
OBJECTS :=main.o engine.o database.o rule.o clause.o
BIN :=main

test: install
	clear
	@./$(BIN) test.rbe test2.rbe

install: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -rf *.o
	rm -rf $(BIN)
