CC = gcc
BUILD_DIR = build

CFLAGS = -Wall -I ./include

SRCS = List.c Graph.c miris.c Map.c
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
EXEC = $(BUILD_DIR)/miris


all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean