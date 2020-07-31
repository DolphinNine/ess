SERVER := echo_server
CLIENT := echo_client
SERVER_SOURCES := server.c
CLIENT_SOURCES := client.c

server_tcp_debug: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lpthread -ansi -pedantic -Wall -D TCP
		@gdb $(SERVER)
		@rm -rf $(SERVER)

server_udp_debug: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lpthread -ansi -pedantic -Wall -D UDP
		@gdb $(SERVER)
		@rm -rf $(SERVER)

client_tcp_debug: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall -D TCP
		@gdb $(CLIENT)
		@rm -rf $(CLIENT)

client_udp_debug: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall -D UDP
		@gdb $(CLIENT)
		@rm -rf $(CLIENT)

bins_tcp: server_tcp client_tcp

bins_udp: server_udp client_udp

server_tcp: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lpthread -ansi -pedantic -Wall -D TCP

server_udp: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lpthread -ansi -pedantic -Wall -D UDP

client_tcp: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall -D TCP

client_udp: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall -D UDP

server_tcp_exec: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lpthread -ansi -pedantic -Wall -D TCP
		@./$(SERVER)
		@rm -rf $(SERVER)

server_udp_exec: $(SERVER_SOURCES)
		@gcc $(SERVER_SOURCES) -o $(SERVER) -lpthread -ansi -pedantic -Wall -D UDP
		@./$(SERVER)
		@rm -rf $(SERVER)

client_tcp_exec: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall -D TCP
		@./$(CLIENT)
		@rm -rf $(CLIENT)

client_udp_exec: $(CLIENT_SOURCES)
		@gcc $(CLIENT_SOURCES) -o $(CLIENT) -lpthread -ansi -pedantic -Wall -D UDP
		@./$(CLIENT)
		@rm -rf $(CLIENT)

.PHONY: clean
clean:
		@rm -rf $(SERVER)
		@rm -rf $(CLIENT)
