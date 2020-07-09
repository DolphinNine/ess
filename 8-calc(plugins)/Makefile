BIN := calc
SOURCES := main.c
HEADERS := proto.h
LIB := calc_lib
LIB_EXT := add subtract multiply divide
LIB_SOURCES := take_var.c libs_init.c
LIB_SOURCES_EXT := add.c subtract.c multiply.c divide.c

.PHONY: all
all: $(LIB) $(LIB_EXT) $(BIN) clean

$(LIB): $(LIB_SOURCES)
		gcc -fPIC -c $(LIB_SOURCES)
		gcc -shared -o lib$(LIB).so *.o

$(LIB_EXT): $(LIB_SOURCES_EXT)
		gcc -fPIC -c $(LIB_SOURCES_EXT) #Как убрать повторы?
		gcc -shared -o lib$@.so $@.o

$(BIN): $(SOURCES)
		gcc $(SOURCES) -L. -l$(LIB) -ldl -o $(BIN)
		gcc -c $(SOURCES)
		gcc main.o -L. -l$(LIB) -ldl -o $(BIN)
		$ LD_LIBRARY_PATH=. ./$(BIN)

		#gcc $(SOURCES) -L. -l$(LIB) -ldl -o $(BIN) -g
		#gcc -c $(SOURCES) -g
		#gcc main.o -L. -l$(LIB) -ldl -o $(BIN) -g
		#$ LD_LIBRARY_PATH=. gdb ./$(BIN)

#$(BIN): $(HEADERS)

.PHONY: clean
clean:
	rm -rf $(BIN)
	rm -rf *.o
	rm -rf *.so
