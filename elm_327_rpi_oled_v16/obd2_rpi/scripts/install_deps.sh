#!/bin/bash
# install_deps.sh — Instalar dependencias para obd2_rpi en Raspberry Pi
#
# Uso:
#   ./scripts/install_deps.sh            # Instalar todo
#   ./scripts/install_deps.sh --check    # Solo verificar
#   ./scripts/install_deps.sh --spi      # Solo habilitar SPI
#   ./scripts/install_deps.sh --bt       # Solo configurar BT

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
err()   { echo -e "${RED}[ERR]${NC} $1"; }

check_rpi() {
    if [ ! -f /proc/device-tree/model ]; then
        warn "No parece ser una Raspberry Pi"
        return 1
    fi
    local model=$(tr -d '\0' < /proc/device-tree/model)
    info "RPi detectada: ${model}"
    return 0
}

check_deps() {
    local missing=0
    local pkgs=(
        "build-essential:gcc"
        "cmake:cmake"
        "git:git"
        "libbluetooth-dev:bluetooth.h"
        "libgpiod-dev:gpiod.h"
        "bluez:bluetoothctl"
    )

    for entry in "${pkgs[@]}"; do
        local pkg="${entry%%:*}"
        local bin="${entry##*:}"
        if dpkg -l "$pkg" &>/dev/null 2>&1; then
            echo -e "  ${GREEN}✓${NC} $pkg"
        else
            echo -e "  ${RED}✗${NC} $pkg (falta)"
            missing=$((missing + 1))
        fi
    done
    return $missing
}

install_pkgs() {
    info "Actualizando lista de paquetes..."
    sudo apt update

    info "Instalando dependencias..."
    sudo apt install -y \
        build-essential \
        cmake \
        git \
        libbluetooth-dev \
        libgpiod-dev \
        libgpiod2 \
        gpiod \
        bluez \
        bluetooth
}

enable_spi() {
    local config="/boot/firmware/config.txt"
    [ ! -f "$config" ] && config="/boot/config.txt"
    [ ! -f "$config" ] && { err "No se encontró config.txt"; return 1; }

    if grep -q "^dtparam=spi=on" "$config" 2>/dev/null; then
        info "SPI ya está habilitado en $config"
    else
        info "Habilitando SPI en $config..."
        echo "dtparam=spi=on" | sudo tee -a "$config" >/dev/null
        warn "SPI habilitado — requiere reinicio: sudo reboot"
    fi

    if [ -c /dev/spidev0.0 ]; then
        info "Dispositivo SPI /dev/spidev0.0 disponible"
    else
        warn "/dev/spidev0.0 no disponible (cargar spi-bcm2835 o reiniciar)"
    fi
}

setup_bt() {
    info "Configurando Bluetooth..."
    sudo systemctl enable bluetooth 2>/dev/null || true
    sudo systemctl start bluetooth  2>/dev/null || true

    if command -v bluetoothctl &>/dev/null; then
        echo ""
        echo "╔══════════════════════════════════════════════╗"
        echo "║  Para emparejar el ELM327:                   ║"
        echo "║                                              ║"
        echo "║  1. bluetoothctl                             ║"
        echo "║  2. power on                                 ║"
        echo "║  3. scan on                                  ║"
        echo "║  4. pair 00:1D:A5:07:23:6E                   ║"
        echo "║  5. trust 00:1D:A5:07:23:6E                  ║"
        echo "║  6. quit                                     ║"
        echo "╚══════════════════════════════════════════════╝"
    else
        err "bluetoothctl no encontrado (bluez no instalado)"
    fi
}

case "${1:-all}" in
    --check|-c)
        info "Verificando dependencias..."
        check_rpi || true
        check_deps || true
        ;;
    --spi)
        enable_spi
        ;;
    --bt)
        setup_bt
        ;;
    all|*)
        info "=== Instalacion de dependencias para obd2_rpi ==="
        check_rpi || true
        echo ""
        install_pkgs
        echo ""
        enable_spi
        echo ""
        setup_bt
        echo ""
        info "Instalacion completada"
        echo ""
        echo "Pasos siguientes:"
        echo "  1. Si se habilito SPI, reiniciar: sudo reboot"
        echo "  2. Compilar: ./scripts/build.sh o scripts/build.sh remote"
        echo "  3. Emparejar ELM327 via bluetoothctl"
        echo "  4. Ejecutar: ./bin/obd2_rpi 00:1D:A5:07:23:6E"
        ;;
esac
