# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./

# Directories
LEXER_DIR = lexer
DYNAMIC_STRING_DIR = dynamicString
ERRORS_DIR = errors

# Source files
SRCS = $(LEXER_DIR)/scanner.c \
       $(DYNAMIC_STRING_DIR)/dynamicString.c \
       $(ERRORS_DIR)/error.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable
EXEC = scanner

# Default target
all: $(EXEC)

# Link the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean
