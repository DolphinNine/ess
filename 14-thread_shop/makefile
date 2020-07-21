BIN := thread_shop
SOURCES:= main.c customer.c loader.c

$(BIN): $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -lpthread
		@./$(BIN)
		@rm -rf $(BIN)

debug: $(SOURCES)
		@gcc $(SOURCES) -o $(BIN) -lpthread -g
		@gdb $(BIN)
		@rm -rf $(BIN)
