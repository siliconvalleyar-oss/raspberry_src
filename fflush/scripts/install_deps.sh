#!/bin/bash
# install_deps.sh — Instalar dependencias para fflush en Raspberry Pi
set -e

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
err()   { echo -e "${RED}[ERR]${NC} $1"; }

check_rpi() {
    [ ! -f /proc/device-tree/model ] && { warn "No es una Raspberry Pi"; return 1; }
    info "RPi: $(tr -d '\0' < /proc/device-tree/model)"
}

install_pkgs() {
    info "Actualizando lista de paquetes..."
    sudo apt update
    info "Instalando build-essential..."
    sudo apt install -y build-essential
}

case "${1:-all}" in
    --check|-c)
        check_rpi || true
        dpkg -l build-essential &>/dev/null && info "build-essential: OK" || warn "build-essential: falta"
        ;;
    all|*)
        info "=== Dependencias fflush ==="
        check_rpi || true
        install_pkgs
        echo ""
        info "Instalacion completa"
        echo "  Compilar: make"
        echo "  Ejecutar: sudo ./flush"
        ;;
esac
