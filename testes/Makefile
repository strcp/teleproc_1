CC = gcc
CFLAGS = -g -W -Wall -I../include
EXEC = listener
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: $(EXEC)

client: $(OBJ)
	@$(CC) -o $@ $^

.PHONY: clean mrproper
clean:
	rm -rf *.o
	rm -rf $(EXEC)

