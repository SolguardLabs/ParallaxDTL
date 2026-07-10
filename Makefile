CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Isrc
BUILD_DIR := build
TARGET := $(BUILD_DIR)/parallaxdtl
SRC := \
	src/main.c \
	src/pdtl_hash.c \
	src/pdtl_journal.c \
	src/pdtl_ledger.c \
	src/pdtl_json.c \
	src/pdtl_scenarios.c

.PHONY: all clean test loc

all: $(TARGET)

$(TARGET): $(SRC) src/pdtl.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRC) -o $(TARGET)

test:
	node --test tests/node/*.test.js

loc:
	node scripts/check-loc.mjs

clean:
	rm -rf $(BUILD_DIR)
