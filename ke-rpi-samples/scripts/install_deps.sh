#!/bin/bash
# Instalación de dependencias para ke-rpi-samples
# Uso: sudo bash scripts/install_deps.sh

echo "=== Instalando dependencias para ke-rpi-samples ==="

apt-get update
apt-get install -y build-essential

echo "=== Instalación completada ==="
echo ""
echo "Asegúrate de habilitar las interfaces en raspi-config:"
echo "  SPI, I2C, UART"
echo ""
echo "Ejecutar con sudo para acceso a /dev y /sys"
