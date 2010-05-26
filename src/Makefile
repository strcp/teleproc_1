CC = gcc
CFLAGS = -g -W -Wall -I../include
EXEC = client router
SRC = $(wildcard *.c)
OBJ = route.o connection.o

all: $(EXEC)

client: client.o $(OBJ)
	$(CC) -o $@ $^

router: router.o $(OBJ)
	$(CC) -o $@ $^

.PHONY: clean mrproper
clean:
	rm -rf *.o
	rm -rf $(EXEC)

