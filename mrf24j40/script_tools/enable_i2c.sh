#!/bin/bash
echo "Habilitando I2C en Raspberry Pi..."
if ! grep -q "dtparam=i2c_arm=on" /boot/config.txt; then
    echo "dtparam=i2c_arm=on" | sudo tee -a /boot/config.txt
fi
sudo raspi-config nonint do_i2c 0
sudo apt-get install -y i2c-tools
echo "I2C habilitado. Reinicia para aplicar cambios."
