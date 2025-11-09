# Makefile pour le projet halffloat
# Note: Compilation stricte avec optimisation maximale par défaut (MinGW-W64 en 64 bits)

CC = gcc
CFLAGS = -fno-aggressive-loop-optimizations -std=c99 -Wall -Wextra -Werror -Wpedantic -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition -Wdeclaration-after-statement -Werror=unused-variable -O3 -march=native -mtune=native -flto -ffast-math -funroll-loops -m64
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
TARGET = main.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o *.exe 2>NUL || exit 0

info:
	@echo "Configuration du compilateur:"
	@$(CC) --version
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  all     - Compile l'executable principal (optimisation maximale par défaut)"
	@echo "  clean   - Nettoie les fichiers objets et executables"
	@echo "  info    - Affiche ces informations"

.PHONY: all clean info
