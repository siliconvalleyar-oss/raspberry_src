#!/bin/bash
# Instalador de dependencias para ALS31313_Pi (Sensor Hall 3D por I2C)
# Uso: sudo bash scripts/install_deps.sh

set -e

echo "============================================"
echo "  Instalando dependencias para ALS31313_Pi"
echo "============================================"

sudo apt update
sudo apt install -y build-essential

# Habilitar I2C
if ! grep -q "dtparam=i2c_arm=on" /boot/firmware/config.txt 2>/dev/null; then
    echo "dtparam=i2c_arm=on" | sudo tee -a /boot/firmware/config.txt
    echo "I2C habilitado en config.txt. Reinicie para aplicar cambios."
else
    echo "I2C ya está habilitado."
fi

echo ""
echo "Instalación completada."
