# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./

# Directories
LEXER_DIR = src/lexer
DYNAMIC_STRING_DIR = src/dynamicString
ERRORS_DIR = src/errors
PARSER_DIR = src/parser
STACK_DIR = src/stack
SYMBOL_TABLE_DIR = src/symTable

OBJ_DIR = obj

# Source files
SRCS = $(LEXER_DIR)/scanner.c \
       $(DYNAMIC_STRING_DIR)/dynamicString.c \
       $(ERRORS_DIR)/error.c \
       $(PARSER_DIR)/parser.c \
       $(STACK_DIR)/stack.c \
       $(SYMBOL_TABLE_DIR)/symTable.c


# Object files
OBJS = $(patsubst src/%, $(OBJ_DIR)/%, $(SRCS:.c=.o))

# Executable
EXEC = scanner

# Default target
all: clean $(EXEC)

# Link the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

# Phony targets
.PHONY: all clean