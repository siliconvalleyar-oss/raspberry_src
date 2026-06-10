CXX = g++
CXXFLAGS = -Iinclude
LDFLAGS = -lbcm2835

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRCS = main.cpp $(SRC_DIR)/ssd1963.cpp
OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))
TARGET = $(BIN_DIR)/ssd1963_app

$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
