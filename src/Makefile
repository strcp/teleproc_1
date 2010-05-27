CC = gcc
CFLAGS = -g -W -Wall -I../include -lpthread
EXEC = client router
SRC = $(wildcard *.c)
OBJ = route.o connection.o cmd_parser.o

all: $(EXEC)

client: client.o $(OBJ)
	$(CC) -o $@ $^

router: router.o $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean mrproper
clean:
	rm -rf *.o
	rm -rf $(EXEC)

