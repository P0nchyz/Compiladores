# Compiler
CC := gcc

# Flags
CFLAGS := -std=c11 -Wall -Wextra -Werror \
          -Wshadow -Wconversion -Wdouble-promotion \
          -Wformat=2 -Wstrict-prototypes -Wmissing-prototypes \
          -O2 -g

CPPFLAGS := -Ilib

# Directories
SRC_DIR := src
LIB_DIR := lib
BUILD_DIR := build

# Find sources
SRC := $(wildcard $(SRC_DIR)/*.c)
LIB_SRC := $(wildcard $(LIB_DIR)/*/*.c)

# Map sources to build objects
OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/$(SRC_DIR)/%.o) \
       $(LIB_SRC:$(LIB_DIR)/%.c=$(BUILD_DIR)/$(LIB_DIR)/%.o)

# Dependency files
DEP := $(OBJ:.o=.d)

# Output binary
TARGET := $(BUILD_DIR)/program.out

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJ) -o $@

# Compile src/
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# Compile lib/
$(BUILD_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEP)

# Debug / relaxed flags
noflags: CFLAGS := -O2 -g
noflags: $(TARGET)

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Rebuild
re: clean all

.PHONY: all clean re noflags
