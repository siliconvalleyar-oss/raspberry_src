#!/bin/bash
# Instalación de dependencias para OpenScope MZ
# Uso: sudo bash scripts/install_deps.sh

echo "=== Instalando dependencias para OpenScope MZ ==="

apt-get update
apt-get install -y python3 python3-serial python3-pip

echo "=== Instalación completada ==="
echo ""
echo "Para instalar dependencias Python:"
echo "  pip3 install -r requirements.txt"
echo ""
echo "O crear un entorno virtual:"
echo "  python3 -m venv openscope_env"
echo "  source openscope_env/bin/activate"
echo "  pip install -r requirements.txt"
