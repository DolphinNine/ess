BIN := calc_dynamic_lib
LIB := calc_dynamic
SOURCES := main.c
LIB_SOURCES := add.c substract.c multiply.c divide.c take_var.c
HEADERS := proto.h

.PHONY: all
all: $(LIB) $(BIN) clean

$(LIB): $(LIB_SOURCES)
		gcc -fPIC -c $(LIB_SOURCES)
		gcc -shared -o lib$(LIB).so *.o

$(BIN): $(SOURCES)
		gcc $(SOURCES) -L. -l$(LIB) -o $(BIN)
		gcc -c $(SOURCES)
		gcc main.o -L. -l$(LIB) -o $(BIN)
		$ LD_LIBRARY_PATH=. ./$(BIN)

#$(BIN): $(HEADERS)

.PHONY: clean
clean:
	rm -rf $(BIN)
	rm -rf *.o
	rm -rf lib$(LIB).so
