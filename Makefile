CC = gcc
CFLAGS = -Wall -ansi -lpthread
EXEC = client router
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all:
	cd src && $(MAKE)

.PHONY: clean mrproper

clean:
	cd src/ && rm -rf *.o *~ $(EXEC)
	rm -rf docs/refs
