SRC_DIR := ./src
OUTPUT = ./fuzzer


SRC := $(shell find $(SRC_DIR) -name '*.c')

CC = gcc

LDFLAGS = -lpthread

all: $(OUTPUT)

$(OUTPUT): $(SRC)
	$(CC) $(SRC) -o $(OUTPUT) $(LDFLAGS)

clean:
	rm $(OUTPUT)