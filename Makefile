CC = gcc
CFLAGS = -Wall
SOURCES = main.c errors.c crc32.c utils.c file_ops.c message.c
OUTPUT = test_is

all: clean $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT)

.PHONY: clean

clean:
	rm -f $(OUTPUT)
