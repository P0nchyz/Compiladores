# Compiler
CC = gcc

# Strict compilation flags
CFLAGS = -std=c11 -Wall -Wextra -Werror \
	 -Wshadow -Wconversion -Wdouble-promotion \
	 -Wformat=2 -Wstrict-prototypes -Wmissing-prototypes \
	 -O2 -g

# Directories
SRC_DIR = src
LIB_DIR = lib

# Include paths
CPPFLAGS = -I$(LIB_DIR)

# Sources
SRC = $(wildcard $(SRC_DIR)/*.c)
LIB_SRC = $(wildcard $(LIB_DIR)/*/*.c)

# Target
TARGET = program

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRC) $(LIB_SRC) -o $(TARGET)

# Clean
clean:
	rm -rf $(TARGET)

# Rebuild
re: clean all

.PHONY: all clean re
