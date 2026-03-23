# Variables de compilation
CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -ldl -lSDL2

# On compile splash et les 4 joueurs par defaut
all: splash p1 p2 p3 p4

splash: src/main.c
	$(CC) $(CFLAGS) src/main.c -o splash $(LDFLAGS)

p1: players/p1.c
	$(CC) $(CFLAGS) -fPIC -shared players/p1.c -o players/p1.so

p2: players/p2.c
	$(CC) $(CFLAGS) -fPIC -shared players/p2.c -o players/p2.so

p3: players/p3.c
	$(CC) $(CFLAGS) -fPIC -shared players/p3.c -o players/p3.so

p4: players/p4.c
	$(CC) $(CFLAGS) -fPIC -shared players/p4.c -o players/p4.so

clean:
	rm -f splash players/*.so