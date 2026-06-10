# ============================================================
# Makefile - OBD-II Monitor for Raspberry Pi
# ELM327 Bluetooth + SSD1306 OLED (I2C)
# ============================================================
#
# Compilación local:
#   make
#   make clean
#
# Compilación remota (Raspberry Pi):
#   ssh joy@raspberry.local "cd /home/joy/src/freebuff/elm327_oled_v1 && make"
#
# Ejecución (en RPi):
#   sudo ./bin/obd_monitor
# ============================================================

# Compilador y flags
CXX      ?= g++
CXXFLAGS  = -std=c++11 -Wall -Wextra -Wpedantic -Wno-unused-parameter
CXXFLAGS += -O2 -mcpu=cortex-a72 -mtune=cortex-a72
CXXFLAGS += -Iinclude -MMD -MP

# Librerías
LDLIBS   = -lbluetooth -lpthread -lrt -lbcm2835

# Directorios
SRC_DIR  = src
INC_DIR  = include
OBJ_DIR  = obj
BIN_DIR  = bin

# Archivos fuente y objeto
SRCS     = $(wildcard $(SRC_DIR)/*.cpp)
OBJS     = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS     = $(OBJS:.o=.d)

# Binario final
TARGET   = $(BIN_DIR)/obd_monitor

# ============================================================
# Reglas
# ============================================================

.PHONY: all clean directories

all: directories $(TARGET)

# Crear directorios necesarios
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Linkear binario final
$(TARGET): $(OBJS)
	@echo "  LD   $@"
	@$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@
	@echo "  -> Binario: $(TARGET)"
	@echo "  -> Tamaño:"
	@ls -lh $(TARGET)

# Compilar archivos fuente
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "  CXX  $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Incluir dependencias automáticas
-include $(DEPS)

# Limpiar
clean:
	@echo "  CLEAN"
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "  -> Limpio!"

# Reconstruir desde cero
rebuild: clean all

# === Meta ===

# Mostrar información del entorno
info:
	@echo "OBD-II Monitor Makefile"
	@echo "  CXX:     $(CXX)"
	@echo "  Flags:   $(CXXFLAGS)"
	@echo "  LDLIBS:  $(LDLIBS)"
	@echo "  Sources: $(SRCS)"
	@echo "  Objects: $(OBJS)"
	@echo "  Target:  $(TARGET)"

# Ayuda
help:
	@echo "Comandos disponibles:"
	@echo "  make          Compilar el proyecto"
	@echo "  make clean    Limpiar archivos generados"
	@echo "  make rebuild  Reconstruir desde cero"
	@echo "  make info     Mostrar información de compilación"
