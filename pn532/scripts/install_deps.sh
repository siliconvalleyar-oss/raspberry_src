#!/bin/bash
# install_deps.sh — Instalar dependencias para PN532 NFC Payment
#
# Uso:
#   ./scripts/install_deps.sh

set -e

echo "=== Instalacion de dependencias para PN532 NFC Payment ==="
echo ""

echo "Actualizando lista de paquetes..."
sudo apt update

echo ""
echo "Instalando dependencias..."
sudo apt install -y \
    build-essential \
    libnfc-dev \
    libcurl4-openssl-dev \
    nlohmann-json-dev

echo ""
echo "=== Instalacion completada ==="
echo ""
echo "Compilar con: make"
echo "Ejecutar con: sudo ./bin/nfc_payment"
