SERVER := mq_server
CLIENT := mq_client
SERVER_SOURCES := server.c
CLIENT_SOURCES := client.c

server_debug: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -ansi -pedantic -Wall -lrt -lpthread -g
		@gdb $(SERVER)
		@rm -rf $(SERVER)

client_debug: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -ansi -pedantic -Wall -lrt -lpthread -g
		@gdb $(CLIENT)
		@rm -rf $(CLIENT)

bins: server client

server: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -ansi -pedantic -Wall -lrt -lpthread

client: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -ansi -pedantic -Wall -lrt -lpthread

server_execute: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -ansi -pedantic -Wall -lrt -lpthread
		@./$(SERVER)
		@rm -rf $(SERVER)

client_execute: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -ansi -pedantic -Wall -lrt -lpthread
		@./$(CLIENT)
		@rm -rf $(CLIENT)

.PHONY: clean
clean:
		@rm -rf $(SERVER)
		@rm -rf $(CLIENT)
