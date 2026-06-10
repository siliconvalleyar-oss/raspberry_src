#!/bin/bash
# Instalación de dependencias para el proyecto GSM SIM800
# Uso: sudo bash scripts/install_deps.sh

echo "=== Instalando dependencias para GSM SIM800 ==="

apt-get update
apt-get install -y minicom picocom

echo "=== Instalación completada ==="
echo ""
echo "Herramientas instaladas: minicom, picocom"
echo "No olvides habilitar la UART en config.txt:"
echo "  enable_uart=1"
echo "  dtoverlay=disable-bt"
