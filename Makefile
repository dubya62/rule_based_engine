
CC :=gcc
CFLAGS :=-O3
OBJECTS :=rbe.o engine.o database.o rule.o clause.o
BIN :=rbe

test: install
	clear
	@./$(BIN) 0 -1 test.rbe test2.rbe

install: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -rf *.o
	rm -rf $(BIN)
