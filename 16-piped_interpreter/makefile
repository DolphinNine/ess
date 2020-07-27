BIN := piped_interpreter
SOURCES:= main.c

.PHONY: all
all: binary_execute clean

debug: $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -g
		@gdb $(BIN)
		@rm -rf $(BIN)

binary: $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -ansi -pedantic -Wall

binary_execute: $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -ansi -pedantic -Wall
		@./$(BIN)

.PHONY: clean
clean: $(SOURCES)
		@rm -rf $(BIN)
