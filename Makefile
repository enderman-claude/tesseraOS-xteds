# xteds build and tests. The bridge needs teds (the Ender display server): set TEDS_REPO to a
# teds checkout (default ../tesseraOS-teds). The integration test links teds' library.
CC     ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -Iinclude

TEDS_REPO ?= ../tesseraOS-teds
TEDS_LIB  := $(TEDS_REPO)/build/libteds.a
TEDS_INC  := -I$(TEDS_REPO)/include

BUILD := build
LIB_SRC := src/window_map.c src/bridge.c

all: $(BUILD)/xteds

$(BUILD):
	mkdir -p $(BUILD)

$(TEDS_LIB):
	$(MAKE) -C $(TEDS_REPO) all

$(BUILD)/xteds: src/xteds_main.c $(LIB_SRC) | $(BUILD)
	$(CC) $(CFLAGS) $(TEDS_INC) $^ $(TEDS_LIB) -o $@

$(BUILD)/test_window_map: tests/test_window_map.c src/window_map.c | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/test_bridge: tests/test_bridge.c $(LIB_SRC) $(TEDS_LIB) | $(BUILD)
	$(CC) $(CFLAGS) $(TEDS_INC) tests/test_bridge.c $(LIB_SRC) $(TEDS_LIB) -o $@

test: $(BUILD)/test_window_map $(BUILD)/test_bridge
	./$(BUILD)/test_window_map
	./$(BUILD)/test_bridge

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
