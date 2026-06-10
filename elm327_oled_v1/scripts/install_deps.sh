#!/bin/bash
# install_deps.sh — Instalar dependencias para elm327_oled_v1 en Raspberry Pi
set -e

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
err()   { echo -e "${RED}[ERR]${NC} $1"; }

check_rpi() {
    [ ! -f /proc/device-tree/model ] && { warn "No es una Raspberry Pi"; return 1; }
    info "RPi: $(tr -d '\0' < /proc/device-tree/model)"
}

install_apt() {
    info "Actualizando lista de paquetes..."
    sudo apt update
    info "Instalando dependencias..."
    sudo apt install -y \
        build-essential \
        libbluetooth-dev \
        libgpiod-dev
}

install_bcm2835() {
    if [ -f /usr/local/lib/libbcm2835.a ]; then
        info "bcm2835 ya instalado en /usr/local/lib"
        return
    fi
    info "Instalando bcm2835 library..."
    local VER="1.75"
    wget -q "http://www.airspayce.com/mikem/bcm2835/bcm2835-${VER}.tar.gz"
    tar xzf "bcm2835-${VER}.tar.gz"
    cd "bcm2835-${VER}"
    ./configure
    make
    sudo make install
    cd ..
    rm -rf "bcm2835-${VER}" "bcm2835-${VER}.tar.gz"
    info "bcm2835 instalado en /usr/local/lib"
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
        dpkg -l libbluetooth-dev &>/dev/null && info "libbluetooth-dev: OK" || warn "libbluetooth-dev: falta"
        dpkg -l libgpiod-dev &>/dev/null && info "libgpiod-dev: OK" || warn "libgpiod-dev: falta"
        [ -f /usr/local/lib/libbcm2835.a ] && info "bcm2835: OK" || warn "bcm2835: falta"
        ;;
    all|*)
        info "=== Dependencias elm327_oled_v1 ==="
        check_rpi || true
        install_apt
        install_bcm2835
        enable_i2c
        echo ""
        info "Instalacion completa"
        echo "  Compilar: make"
        echo "  Ejecutar: sudo ./bin/obd_monitor"
        echo "  Si se habilito I2C, reiniciar: sudo reboot"
        ;;
esac
