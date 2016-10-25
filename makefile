server: server.h server.c main_server.c
	gcc server.c main_server.c -o server -lpthread

client: client_core utils sender main_client receiver
	gcc client_core.o utils.o sender.o main_client.o receiver.o -o client -lpthread

client_core: client.h client.c
	gcc -c client.c -o client_core.o

main_client: main_client.c
	gcc -c main_client.c -o main_client.o

debug_client: client_core utils sender main_client receiver
	gcc -g client_core.o utils.o sender.o main_client.o receiver.o -o client_dbg -lpthread

sender: sender.h sender.c
	gcc -c  sender.c -o sender.o

utils: utils.h utils.c
	gcc -c utils.c -o utils.o

receiver: receiver.h receiver.c
	gcc -c receiver.c -o receiver.o
run_client: client
	./client

clean:
	rm client.o client server utils.o sender.o