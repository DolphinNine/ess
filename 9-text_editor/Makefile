BIN := editor
SOURCES:= main.c
#HEADERS := proto.h

$(BIN): $(SOURCES)
		gcc $(SOURCES) -o $(BIN) -lncurses
		./$(BIN)
		rm -rf $(BIN)
$(BIN): $(HEADERS)
