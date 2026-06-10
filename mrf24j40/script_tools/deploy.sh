#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

print_header() { echo -e "\n=================================================\n  $1\n================================================="; }

check_spi() {
    print_header "Verificando SPI"
    if [ ! -e /dev/spidev0.0 ] && [ ! -e /dev/spidev0.1 ]; then
        echo "ERROR: SPI no habilitado. Ejecute: sudo raspi-config"
        exit 1
    fi
    echo "SPI disponible ✓"
}

check_compiler() {
    print_header "Verificando compilador"
    if ! command -v g++ &>/dev/null; then
        sudo apt-get update -q && sudo apt-get install -y -q g++ build-essential
    fi
    echo "g++: $(g++ --version | head -1)"
}

select_role() {
    print_header "Seleccionar rol"
    echo "  1) TRANSMISOR (Raspberry A)"
    echo "  2) RECEPTOR (Raspberry B)"
    read -p "Opción [1/2]: " role
    case "$role" in
        1) PROJECT="mrf24_tx"; BINARY="mrf24_transmitter" ;;
        2) PROJECT="mrf24_rx"; BINARY="mrf24_receiver" ;;
        *) exit 1 ;;
    esac
}

check_spi
check_compiler
select_role

cd "$SCRIPT_DIR/$PROJECT"
make clean && make
echo -e "\nBinario: $PROJECT/bin/$BINARY"
echo "Ejecutar: sudo ./$PROJECT/bin/$BINARY"
