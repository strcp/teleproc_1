CC = gcc
CFLAGS = -g -W -Wall -I../include -lpthread -lreadline -lncurses
EXEC = client router
SRC = $(wildcard *.c)
OBJ = data.o route.o connection.o cmd_parser.o

all: $(EXEC)

client: client.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

router: router.o $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean mrproper
clean:
	rm -rf *.o
	rm -rf $(EXEC)

