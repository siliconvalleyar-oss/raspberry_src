#!/bin/bash
# Instalador de dependencias para 25aa02e48
# Uso: sudo bash scripts/install_deps.sh

set -e

echo "============================================"
echo "  Instalando dependencias para 25aa02e48"
echo "============================================"

# Paquetes base
sudo apt update
sudo apt install -y build-essential

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
