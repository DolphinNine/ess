BIN := manager
SOURCES:= main.c get_dir.c #print_dir.c

$(BIN): $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -lncurses -g
		@./$(BIN)
		@rm -rf $(BIN)
