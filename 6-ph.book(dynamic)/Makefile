BIN := struct_pass
SOURCES:= main.c show_all.c search_name.c search_number.c modify_entry.c erase_entry.c add_entry.c quit.c
HEADERS := proto.h defs.h

$(BIN): $(SOURCES)
		gcc $(SOURCES) -o $(BIN)
		./$(BIN)
		rm -rf $(BIN)
$(BIN): $(HEADERS)
