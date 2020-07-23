BIN := manager
SOURCES:= main.c get_req.c quit.c draw_win.c print_message.c cp_routine.c

$(BIN): $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -lncurses -lmagic -lpthread
		@./$(BIN)
		@rm -rf $(BIN)

debug: $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -lncurses -lmagic -lpthread -g
		@gdb $(BIN)
		@rm -rf $(BIN)
