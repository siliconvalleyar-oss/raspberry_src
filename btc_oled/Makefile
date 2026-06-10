SRC_APP  = src/OLED_BTC
SRC_SSD  = src
INC      = include
OBJ      = obj
BIN      = bin
TARGET   = $(BIN)/app_oled

CC       = g++
CFLAGS   = -Wall -Wextra -std=c++17 -O2 -I$(INC) -I./include/json
LDFLAGS  = -lbcm2835 -lrt -lcurl

SRCS     = $(wildcard $(SRC_APP)/*.cpp) \
           $(SRC_SSD)/SSD1306_OLED.cpp \
           $(SRC_SSD)/SSD1306_OLED_graphics.cpp \
           $(SRC_SSD)/SSD1306_OLED_Print.cpp \
           $(SRC_SSD)/SSD1306_OLED_font.cpp
OBJS     = $(addprefix $(OBJ)/, $(notdir $(SRCS:.cpp=.o)))

vpath %.cpp $(SRC_APP):$(SRC_SSD)

.PHONY: all run clean install

all: pre-build $(TARGET)

run:
	sudo $(TARGET)

pre-build:
	@mkdir -p $(BIN) $(OBJ)

$(OBJ)/%.o: %.cpp
	@echo '[CC] $<'
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	@echo '[LINK] $@'
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo '[DONE] bin/app_oled'

clean:
	rm -vf $(OBJ)/*.o $(TARGET)

install:
	@echo '[INSTALL] Dependencias...'
	sudo apt update
	sudo apt install -y libcurl4-openssl-dev
	@if [ ! -f /usr/local/lib/libbcm2835.a ]; then \
		echo "Instalando bcm2835..."; \
		wget -q http://www.airspayce.com/mikem/bcm2835/bcm2835-1.75.tar.gz; \
		tar xzf bcm2835-1.75.tar.gz; \
		cd bcm2835-1.75 && ./configure && make && sudo make install; \
		cd .. && rm -rf bcm2835-1.75*; \
	fi
	@echo '[INSTALL] Completo'
