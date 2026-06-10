#!/bin/bash
# install_deps.sh — Instalar dependencias para ds2482 en Raspberry Pi
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
    info "Instalando libi2c-dev..."
    sudo apt install -y libi2c-dev
}

enable_i2c() {
    local config="/boot/firmware/config.txt"
    [ ! -f "$config" ] && config="/boot/config.txt"
    if grep -q "^dtparam=i2c_arm=on" "$config" 2>/dev/null; then
        info "I2C ya habilitado"
    else
        info "Habilitando I2C en $config..."
        echo "dtparam=i2c_arm=on" | sudo tee -a "$config" >/dev/null
        warn "I2C habilitado — requiere reinicio: sudo reboot"
    fi
    if [ -c /dev/i2c-1 ]; then
        info "Dispositivo I2C disponible"
    else
        warn "/dev/i2c-1 no disponible (cargar i2c-bcm2708 o reiniciar)"
    fi
}

case "${1:-all}" in
    --check|-c)
        check_rpi || true
        dpkg -l libi2c-dev &>/dev/null && info "libi2c-dev: OK" || warn "libi2c-dev: falta"
        ;;
    all|*)
        info "=== Dependencias ds2482 ==="
        check_rpi || true
        install_pkgs
        enable_i2c
        echo ""
        info "Instalacion completa"
        echo "  Compilar: make"
        echo "  Ejecutar: sudo ./ds1994_reader"
        echo "  Si se habilito I2C, reiniciar: sudo reboot"
        ;;
esac
