#!/bin/bash
# Instalación de dependencias para MRF24J40 Radio + OLED
# Uso: sudo bash scripts/install_deps.sh

echo "=== Instalando dependencias para MRF24J40 ==="

apt-get update
apt-get install -y libbcm2835-dev libmysqlcppconn-dev \
                   libmosquitto-dev libqrencode-dev libpng-dev zlib1g-dev

echo "=== Instalación completada ==="
echo ""
echo "Librerías instaladas:"
echo "  - libbcm2835-dev, libmysqlcppconn-dev"
echo "  - libmosquitto-dev, libqrencode-dev, libpng-dev"
echo ""
echo "Además se requiere la librería SSD1306_OLED_RPI:"
echo "  git clone https://github.com/gavinlyonsrepo/SSD1306_OLED_RPI.git"
echo "  cd SSD1306_OLED_RPI && make && sudo make install"
echo ""
echo "Compilar con: make"
