#!/bin/bash
# Instalador de dependencias para dino (Chrome Dino Arcade ST7789)
# Uso: sudo bash scripts/install_deps.sh

set -e

echo "============================================"
echo "  Instalando dependencias para dino"
echo "============================================"

sudo apt update
sudo apt install -y build-essential libgpiod-dev libpthread-stubs0-dev

# Nota: bcm2835 no está en los repos oficiales de Raspberry Pi OS.
# Si necesita la librería bcm2835, descárguela desde:
#   http://www.airspayce.com/mikem/bcm2835/
# y compílela manualmente.

# Habilitar SPI
if ! grep -q "dtparam=spi=on" /boot/firmware/config.txt 2>/dev/null; then
    echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
    echo "SPI habilitado en config.txt. Reinicie para aplicar cambios."
else
    echo "SPI ya está habilitado."
fi

echo ""
echo "Instalación completada."
echo "Si modificó config.txt, reinicie: sudo reboot"
