#!/bin/bash
# install_deps.sh — Instalar dependencias para epaper_display_rpi_2w en Raspberry Pi
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
    sudo apt install -y build-essential cmake
}

enable_spi() {
    local config="/boot/firmware/config.txt"
    [ ! -f "$config" ] && config="/boot/config.txt"
    if grep -q "^dtparam=spi=on" "$config" 2>/dev/null; then
        info "SPI ya habilitado"
    else
        info "Habilitando SPI en $config..."
        echo "dtparam=spi=on" | sudo tee -a "$config" >/dev/null
        warn "SPI habilitado — requiere reinicio: sudo reboot"
    fi
    if [ -c /dev/spidev0.0 ]; then
        info "Dispositivo SPI disponible"
    else
        warn "/dev/spidev0.0 no disponible (cargar spi-bcm2708 o reiniciar)"
    fi
}

case "${1:-all}" in
    --check|-c)
        check_rpi || true
        dpkg -l build-essential &>/dev/null && info "build-essential: OK" || warn "build-essential: falta"
        ;;
    all|*)
        info "=== Dependencias epaper_display_rpi_2w ==="
        check_rpi || true
        install_pkgs
        enable_spi
        echo ""
        info "Instalacion completa"
        echo "  Compilar: make o cmake"
        echo "  Ejecutar: sudo ./app_epaper"
        echo "  Si se habilito SPI, reiniciar: sudo reboot"
        ;;
esac
