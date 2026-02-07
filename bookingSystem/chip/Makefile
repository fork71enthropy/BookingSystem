CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lSDL2

TARGET = chip8
SOURCE = chip8_emulator.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) rom.ch8

.PHONY: all clean run
