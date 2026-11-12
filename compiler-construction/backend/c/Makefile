CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2

SOURCES := $(wildcard src/*.c)
TARGET := c-compiler

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
