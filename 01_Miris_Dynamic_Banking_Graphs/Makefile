CC = gcc
BUILD_DIR = build
SRC_DIR = src

CFLAGS = -Wall -I ./include

SRCS = $(SRC_DIR)/List.c $(SRC_DIR)/Graph.c $(SRC_DIR)/miris.c $(SRC_DIR)/Map.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
EXEC = $(BUILD_DIR)/miris

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean