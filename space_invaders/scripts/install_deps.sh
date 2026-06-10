#!/bin/bash
# install_deps.sh — Instalar dependencias para Space Invaders (ST7789)
#
# Uso:
#   ./scripts/install_deps.sh

set -e

echo "=== Instalacion de dependencias para Space Invaders ==="
echo ""

echo "Actualizando lista de paquetes..."
sudo apt update

echo ""
echo "Instalando dependencias..."
sudo apt install -y \
    build-essential \
    libbcm2835-dev \
    libgpiod-dev

echo ""
echo "=== Instalacion completada ==="
echo ""
echo "Compilar con: make"
echo "Ejecutar con: sudo make run"
