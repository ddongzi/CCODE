CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -g 
LDFLAGS=-lpthread -lssl -lcrypto

CACHE_SERVER_SRC=cache_server.c signal_handler.c net_utils.c log.c
CLUSTER_MANAGER_SRC=cluster_manager.c cJSON.c net_utils.c log.c
CACHE_SERVER_OBJ=$(CACHE_SERVER_SRC:.c=.o)
CLUSTER_MANAGER_OBJ=$(CLUSTER_MANAGER_SRC:.c=.o)
CACHE_SERVER_EXEC=cache_server
CLUSTER_MANAGER_EXEC=cluster_manager

.PHONY: all clean

all: $(CACHE_SERVER_EXEC) $(CLUSTER_MANAGER_EXEC)

$(CACHE_SERVER_EXEC): $(CACHE_SERVER_OBJ)
	$(CC) -o $@ $(CACHE_SERVER_OBJ) $(CFLAGS) $(LDFLAGS)

$(CLUSTER_MANAGER_EXEC): $(CLUSTER_MANAGER_OBJ)
	$(CC) -o $@ $(CLUSTER_MANAGER_OBJ) $(CFLAGS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CACHE_SERVER_OBJ) $(CLUSTER_MANAGER_OBJ) $(CACHE_SERVER_EXEC) $(CLUSTER_MANAGER_EXEC)
