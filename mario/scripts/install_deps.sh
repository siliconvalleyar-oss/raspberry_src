#!/bin/bash
# Instalación de dependencias para Mario Bros Arcade
# Uso: sudo bash scripts/install_deps.sh

echo "=== Instalando dependencias para Mario Bros Arcade ==="

apt-get update
apt-get install -y libbcm2835-dev libgpiod-dev

echo "=== Instalación completada ==="
echo ""
echo "Compilar con: make"
echo "Ejecutar con: sudo make run"
