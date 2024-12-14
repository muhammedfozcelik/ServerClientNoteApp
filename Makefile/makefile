C = gcc
CFLAGS = -Wall -g
LDFLAGS = -lws2_32

SRC_DIR = src
BIN_DIR = bin

all: $(BIN_DIR)/server $(BIN_DIR)/client

$(BIN_DIR)/server: $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN_DIR)/client: $(SRC_DIR)/client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(BIN_DIR)/*