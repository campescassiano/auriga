CC = gcc
CFLAGS = -Wall -fstack-protector -Wextra -Wundef -Wshadow -Wpointer-arith \
         -Wcast-align -Wstrict-prototypes -Wcast-qual -Wswitch-default \
         -Wswitch-enum -Wunreachable-code -g
LDFLAGS = -lz
SOURCES = main.c errors.c crc32.c utils.c file_ops.c message.c
OUTPUT = test_is

all: clean $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OUTPUT)
