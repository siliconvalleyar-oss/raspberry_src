#!/bin/bash
# Instalador de dependencias para ds1994 (iButton 1-Wire)
# Uso: sudo bash scripts/install_deps.sh

set -e

echo "============================================"
echo "  Instalando dependencias para ds1994"
echo "============================================"

sudo apt update
sudo apt install -y build-essential

# Cargar módulos del kernel para 1-Wire
sudo modprobe w1-gpio
sudo modprobe w1-therm

# Habilitar 1-Wire en config.txt
if ! grep -q "dtoverlay=w1-gpio" /boot/firmware/config.txt 2>/dev/null; then
    echo "dtoverlay=w1-gpio" | sudo tee -a /boot/firmware/config.txt
    echo "1-Wire habilitado en config.txt. Reinicie para aplicar cambios."
else
    echo "1-Wire ya está habilitado."
fi

echo ""
echo "Instalación completada."
echo "Si modificó config.txt, reinicie: sudo reboot"
