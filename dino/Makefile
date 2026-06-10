# ============================================================
#  Makefile — Chrome Dino Arcade / RPi Zero 2W
#
#  Compilar:   make
#  Ejecutar:   sudo ./bin/dino
#  Limpiar:    make clean
#  Verificar:  make check
# ============================================================

CXX       ?= g++
CXXFLAGS  ?= -O2 -std=c++14 -Wall -Wextra -Wpedantic
INCLUDES  := -Iinclude
LDFLAGS   := -lpthread

SRCDIR    := src
OBJDIR    := obj
BINDIR    := bin

SRCS      := $(wildcard $(SRCDIR)/*.cpp)
OBJS      := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
TARGET    := $(BINDIR)/dino

.PHONY: all clean check directories

all: directories $(TARGET)
	@echo ""
	@echo "  Build OK — ejecutar con:  sudo ./$(TARGET)"
	@echo ""

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

directories: $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "  Clean OK"

check:
	@echo "--- Verificando hardware ---"
	@ls /dev/spidev0.0 2>/dev/null && echo "  [OK] SPI habilitado" || echo "  [ERR] Habilitar: dtparam=spi=on en /boot/firmware/config.txt"
	@ls /dev/gpiochip0 2>/dev/null && echo "  [OK] GPIO disponible" || echo "  [ERR] No se detecto gpiochip0"
	@echo ""
