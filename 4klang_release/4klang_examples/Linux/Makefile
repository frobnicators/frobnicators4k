.PHONY: all clean final ibhgen

CC = gcc
CFLAGS = -O1 -fno-stack-limit -ffast-math -fno-inline -Wall -Wextra -D_GNU_SOURCE=1 -std=c99
LDFLAGS= -s -lSDL -lGL -lGLEW -lpthread

BIN = player
OBJ = player.o 4klang.o

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(BIN) player.o
