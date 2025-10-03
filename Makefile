# Makefile pour le projet halffloat

CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = main.c hf_common.c hf_lib.c hf_precalc.c hf_tests.c
OBJ = $(SRC:.c=.o)
TARGET = main.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o *.exe 2>NUL || exit 0

.PHONY: all clean
