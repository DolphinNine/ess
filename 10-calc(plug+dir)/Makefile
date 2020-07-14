BIN := calc
SOURCES := main.c
LIB := calc_lib
LIB_EXT := add subtract multiply divide
LIB_SOURCES := take_var.c libs_init.c
LIB_SOURCES_EXT := add.c subtract.c multiply.c divide.c

.PHONY: all
all: $(LIB) $(BIN) clean
libs: $(LIB_EXT) #выведенно в отдельную рутину для проверки поведения, при удалении библиотек

$(LIB): $(LIB_SOURCES)
		@gcc -fPIC -c $(LIB_SOURCES)
		@gcc -shared -o libs/lib$(LIB).so *.o

$(LIB_EXT): $(LIB_SOURCES_EXT)
		@gcc -fPIC -c $(LIB_SOURCES_EXT) #Как убрать повторы?
		@gcc -shared -o libs/lib$@.so $@.o
		@rm -rf *.o

$(BIN): $(SOURCES)
		@gcc $(SOURCES) -L libs -l$(LIB) -ldl -o $(BIN)
		@gcc -c $(SOURCES)
		@gcc main.o -L libs -l$(LIB) -ldl -o $(BIN)
		@$ LD_LIBRARY_PATH=libs ./$(BIN)
		@#указание пути используется только для одной основной библиотеки libcalc_lib
		@#все подгруженные из программы библиотеки всё ещё ищутся самой программой

.PHONY: clean
clean:
	@rm -rf $(BIN)
	@rm -rf *.o

clean_libs:
	@rm -rf libs/*.so
