all:
	gcc ksocket.c user1.c -lpthread -o user1
	gcc ksocket.c user2.c -lpthread -o user2