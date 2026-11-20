
CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -I/usr/local/include

LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = src/main.c \
      src/core/game.c \
      src/systems/player.c \
      src/scenes/room.c \
      src/minigames/cipher/cipher.c \
      src/minigames/doodle/doodle.c \
      src/minigames/terminal/terminal.c

TARGET = build/logiccore

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
