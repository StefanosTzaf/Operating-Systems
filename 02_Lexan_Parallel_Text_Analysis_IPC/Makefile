CC = gcc
BUILD_DIR = build

CFLAGS = -Wall -I ./include

# Source files
ROOT_SRC = src/root.c src/Set.c src/rootUtils.c src/types.c
SPLITTER_SRC = src/List.c src/Map.c src/splitter.c src/splitterUtils.c src/types.c
BUILDER_SRC = src/List.c src/Map.c src/builder.c src/builderUtils.c src/types.c

# Object files in build dir
ROOT_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(ROOT_SRC:.c=.o)))
SPLITTER_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SPLITTER_SRC:.c=.o)))
BUILDER_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(BUILDER_SRC:.c=.o)))

# Executable names (ΤΩΡΑ ΒΓΑΙΝΟΥΝ ΣΤΟΝ ΚΕΝΤΡΙΚΟ ΦΑΚΕΛΟ)
ROOT_EXEC = lexan
SPLITTER_EXEC = splitter
BUILDER_EXEC = builder

# Default target: 3 executables
all: $(ROOT_EXEC) $(SPLITTER_EXEC) $(BUILDER_EXEC)

# Ensure build dir exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Pattern rule for all .o files in build dir
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create the root executable
$(ROOT_EXEC): $(ROOT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ROOT_OBJS)

# Rule to create the splitter executable
$(SPLITTER_EXEC): $(SPLITTER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SPLITTER_OBJS)

# Rule to create the builder executable
$(BUILDER_EXEC): $(BUILDER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(BUILDER_OBJS)

# Clean up object files and executables
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ROOT_EXEC) $(SPLITTER_EXEC) $(BUILDER_EXEC)

.PHONY: all clean