TARGET = mario
CC = g++
CFLAGS = -O2 -std=c++11 -Wall -Iinclude
LDFLAGS = -lpthread

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET_BIN = $(BINDIR)/$(TARGET)

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "✓ Compilación exitosa: $(TARGET_BIN)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*.o $(BINDIR)/$(TARGET)

run: $(TARGET_BIN)
	sudo $(TARGET_BIN)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET_BIN)
	sudo $(TARGET_BIN)

help:
	@echo "make        - compilar"
	@echo "make run    - ejecutar (sudo)"
	@echo "make clean  - limpiar"
.PHONY: all clean run debug help
