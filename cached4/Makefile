CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -g -lpthread

SRC=cache_server.c signal_handler.c
OBJ=$(SRC:.c=.o)
EXEC=cache_server

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
