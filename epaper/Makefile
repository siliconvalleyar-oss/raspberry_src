#CC = clang++
CC = g++
#CC = arm-none-eabi-g++
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g
LIBRARIES = -pthread  -lqrencode #-lbcm2835 
SRC_DIR = src
LIB_DIR = libs
OBJ_DIR = obj
BIN_DIR = bin
SRCS = 		$(wildcard $(SRC_DIR)/*.cpp)
LIB_DIRS = 	$(wildcard $(LIB_DIR)/*)
LIB_SRCS = 	$(foreach dir,$(LIB_DIRS),$(wildcard $(dir)/*.cpp))
OBJS = 		$(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cpp=.o) $(notdir $(LIB_SRCS:.cpp=.o))))
N_APP = epaper_app

# Nombre predeterminado para el binario
DEFAULT_TARGET = ${N_APP}

# Obtener el nombre del objetivo desde la línea de comandos
TARGET ?= $(DEFAULT_TARGET)

# Cambiar el nombre del binario según el objetivo
ifeq ($(TARGET), tx)
    APP = $(BIN_DIR)/${N_APP}_tx
else ifeq ($(TARGET), rx)
    APP = $(BIN_DIR)/${N_APP}_rx
else
    APP = $(BIN_DIR)/$(DEFAULT_TARGET)
endif

.PHONY: all clean run

all: $(APP)

# Compilar el binario final
$(APP): $(OBJS) | $(BIN_DIR)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBRARIES)
	@echo "Binary created: $@"

# Compilar archivos de código fuente en SRC_DIR
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@ -MMD -MP
	@echo "Compiled: $< -> $@"

# Plantilla de compilación para subdirectorios de LIB_DIR
define compile_template
$(OBJ_DIR)/%.o: $(LIB_DIR)/$(1)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $$< -o $$@ -MMD -MP
	@echo "Compiled library: $$< -> $$@"
endef

# Aplicar la plantilla a los subdirectorios de LIB_DIR
$(foreach libdir,$(LIB_DIRS),$(eval $(call compile_template,$(notdir $(libdir)))))

# Crear directorios si no existen (sin modificarlos si ya existen)
$(BIN_DIR) $(OBJ_DIR):
	@test -d $@ || mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	rm -f log/*

run: $(APP)
	sudo $<

# Incluir archivos de dependencias generados automáticamente
-include $(OBJS:.o=.d)
