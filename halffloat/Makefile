# Makefile pour le projet halffloat (compatible Unix et Windows)

# Outils et options
CC = gcc
CFLAGS ?= -fno-aggressive-loop-optimizations -std=c99 -Wall -Wextra -Werror -Wpedantic -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition -Wdeclaration-after-statement -Werror=unused-variable -O3 -march=native -mtune=native -ffast-math -funroll-loops -m64
LTO ?= -flto=1

# Sources/objets
SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

# Plateforme (détecte Windows cmd / MinGW via la variable d'environnement OS ou COMSPEC)
is_windows :=
ifeq ($(OS),Windows_NT)
	is_windows := 1
endif
ifneq ($(COMSPEC),)
	is_windows := 1
endif

ifeq ($(is_windows),1)
	EXEEXT := .exe
	RM := del /Q
	NULL := NUL
else
	EXEEXT :=
	RM := rm -f
	NULL := /dev/null
endif

TARGET := main$(EXEEXT)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Direct build shortcut (useful on Windows when 'make' is not available)
build-gcc:
	@echo "Building with direct gcc..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	@echo "Cleaning..."
	-$(RM) $(OBJ) $(TARGET) 2>$(NULL) || true

info:
	@echo "Configuration du compilateur:"
	@$(CC) --version || true
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  all     - Compile l'executable principal"
	@echo "  clean   - Nettoie les fichiers objets et executables"
	@echo "  info    - Affiche ces informations"

.PHONY: all clean info build-gcc

release: CFLAGS += $(LTO)
release: clean all
