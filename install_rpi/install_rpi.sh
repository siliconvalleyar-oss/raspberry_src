#!/bin/bash

echo "Actualizando repositorios..."
sudo apt update

PAQUETES=(
git
curl
wget
nano
htop
tree
unzip
zip
python3
python3-pip
python3-dev
python3-virtualenv
openssh-server
net-tools
avahi-daemon
nmap
mariadb-server
default-mysql-client
)

echo "=============================="
echo " Instalación de paquetes"
echo "=============================="

for paquete in "${PAQUETES[@]}"
do
    echo "➡️ Instalando: $paquete"
    sudo apt install -y "$paquete"

    if [ $? -eq 0 ]; then
        echo "✅ $paquete instalado correctamente"
    else
        echo "❌ Error instalando $paquete"
    fi

    echo "------------------------------"
done

echo "Finalizado 🚀"

