# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
INCLUDES = -Iinclude

# Source files
SRCS = src/main.c src/parser.c src/executor.c src/builtins.c src/history.c src/config.c src/prompt.c src/utils.c

# Object files
OBJS = $(SRCS:.c=.o)

# Libraries
LIBS = -lreadline

# Output binary
TARGET = kali_shell

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
