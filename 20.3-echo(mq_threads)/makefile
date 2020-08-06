SERVER := echo_mq_thread_server
CLIENT := echo_mq_thread_client
SERVER_SOURCES := server.c
CLIENT_SOURCES := client.c

server_debug: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lpthread -lrt -g
		@gdb $(SERVER)
		@rm -rf $(SERVER)

client_debug: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -g
		@gdb $(CLIENT)
		@rm -rf $(CLIENT)

bins: server client

server: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lrt -lpthread -ansi -pedantic -Wall

client: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall

server_exec: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lrt -lpthread -ansi -pedantic -Wall
		@./$(SERVER)
		@rm -rf $(SERVER)

client_exec: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall
		@./$(CLIENT)
		@rm -rf $(CLIENT)

.PHONY: clean
clean:
		@rm -rf $(SERVER)
		@rm -rf $(CLIENT)
