CC = gcc
CFLAGS = -Wall -ansi
	EXEC = client
	SRC = $(wildcard *.c)
	OBJ = $(SRC:.c=.o)

all:
	cd src/client && $(MAKE)

.PHONY: clean mrproper

clean:
	cd src/client && rm -rf *.o *~ $(EXEC)

