#!/bin/bash
# install_deps.sh — Instalar dependencias para Waveforms Live Web App
#
# Uso:
#   ./scripts/install_deps.sh

set -e

echo "=== Instalacion de dependencias para Waveforms Live ==="
echo ""

echo "Actualizando lista de paquetes..."
sudo apt update

echo ""
echo "Instalando Node.js y npm..."
sudo apt install -y nodejs npm

echo ""
echo "Instalando Cordova..."
sudo npm install -g cordova

echo ""
echo "=== Instalacion completada ==="
echo ""
echo "Verificar instalacion:"
echo "  node --version"
echo "  npm --version"
echo "  cordova --version"
echo ""
echo "Desarrollo:"
echo "  cd www && python3 -m http.server 8000"
echo ""
echo "Build Cordova:"
echo "  cordova platform add android"
echo "  cordova build android"
