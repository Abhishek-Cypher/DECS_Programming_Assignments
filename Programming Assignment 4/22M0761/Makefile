
CC = g++ -Wno-write-strings
SERVER_FILE = simple_server.cpp
HTTP_SERVER_FILE = http_server.cpp

all: server

server: $(SERVER_FILE) $(HTTP_SERVER_FILE) load_gen
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -ggdb3 -lpthread -o server

load_gen: load_gen.c
	gcc load_gen.c -ggdb3 -lpthread -o load_gen

clean:
	rm -f server load_gen
	
