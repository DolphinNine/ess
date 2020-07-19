BIN := manager
SOURCES:= main.c get_req.c quit.c draw_win.c

$(BIN): $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -lncurses -lmagic
		@./$(BIN)
		@rm -rf $(BIN)

debug: $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -lncurses -lmagic -g
		@gdb $(BIN)
		@rm -rf $(BIN)
