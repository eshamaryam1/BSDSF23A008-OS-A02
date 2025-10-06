# Makefile for ls utility project

# Compiler and flags

CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Directories

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Target executable

TARGET = $(BIN_DIR)/ls

# Source and object files

SRC = $(SRC_DIR)/ls-v1.2.0.c
OBJ = $(OBJ_DIR)/ls-v1.2.0.o

# Default rule

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
		@mkdir -p $(OBJ_DIR)
		$(CC) $(CFLAGS) -c $< -o $@

# Clean build files

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

# Rebuild everything

rebuild: clean all

.PHONY: all clean rebuild
