server: server_core utils sender main_server  receiver
	g++ server_core.o main_server.o utils.o sender.o receiver.o -o server -lpthread

client: client_core utils sender main_client receiver
	g++ client_core.o utils.o sender.o main_client.o receiver.o -o client -lpthread

client_core: client.h client.c
	g++ -c client.c -o client_core.o

server_core: server.h server.c
	g++ -c server.c -o server_core.o

main_client: main_client.c
	g++ -c main_client.c -o main_client.o

main_server: main_server.c
	g++ -c main_server.c -o main_server.o

debug_client: client_core utils sender main_client receiver
	g++ -g client_core.o utils.o sender.o main_client.o receiver.o -o client_dbg -lpthread

sender: sender.h sender.c
	g++ -c  sender.c -o sender.o

utils: utils.h utils.c
	g++ -c utils.c -o utils.o

receiver: receiver.h receiver.c
	g++ -c receiver.c -o receiver.o
run_client: client
	./client
run_server: server
	./server

clean:
	rm client.o client server utils.o sender.o