# xteds build and test.
CC     ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -Iinclude

BUILD := build
LIB_SRC := src/window_map.c

all: $(BUILD)/xteds

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/xteds: src/xteds_main.c $(LIB_SRC) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/test_window_map: tests/test_window_map.c $(LIB_SRC) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

test: $(BUILD)/test_window_map
	./$(BUILD)/test_window_map

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
