BIN := calc_static_lib
LIB := calc_static
SOURCES := main.c
LIB_SOURCES := add.c substract.c multiply.c divide.c take_var.c
HEADERS := proto.h

.PHONY: all
all: $(LIB) $(BIN) clean

$(LIB): $(LIB_SOURCES)
		gcc -c $(LIB_SOURCES)
		ar rc lib$(LIB).a *.o

$(BIN): $(SOURCES)
		gcc $(SOURCES) -L. -l$(LIB) -o $(BIN)
		./$(BIN)

#$(BIN): $(HEADERS)

.PHONY: clean
clean:
	rm -rf $(BIN)
	rm -rf *.o
	rm -rf lib$(LIB).a
