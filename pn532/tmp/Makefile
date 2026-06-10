# Nombre del ejecutable
TARGET = nfc_payment

# Compilador
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

# Librerías necesarias
LIBS = -lnfc -lcurl

# Directorios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

# Archivos fuente y objetos
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Regla principal
all: $(BIN_DIR)/$(TARGET)

# Link final en bin/
$(BIN_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compilación de cada .cpp -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/*.hpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar todo
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Ejecutar directamente
run: all
	./$(BIN_DIR)/$(TARGET)

.PHONY: all clean run
