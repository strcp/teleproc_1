CC = gcc
CFLAGS = -Wall -ansi
	EXEC = client
	SRC = $(wildcard *.c)
	OBJ = $(SRC:.c=.o)

all:
	cd src && $(MAKE)

.PHONY: clean mrproper

clean:
	cd src/ && rm -rf *.o *~ $(EXEC)

