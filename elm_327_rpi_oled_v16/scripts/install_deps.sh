#!/bin/bash
# install_deps.sh — Instalar dependencias para el proyecto ELM327 OBD-II + OLED
#
# Este script ejecuta el instalador de dependencias dentro de obd2_rpi/
#
# Uso:
#   ./scripts/install_deps.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
OBD2_DIR="$PROJECT_DIR/obd2_rpi"

echo "=== Instalacion de dependencias para ELM327 OBD-II + OLED ==="
echo ""

if [ -f "$OBD2_DIR/scripts/install_deps.sh" ]; then
    echo "Ejecutando instalador de obd2_rpi..."
    bash "$OBD2_DIR/scripts/install_deps.sh" "$@"
else
    echo "Error: No se encontro obd2_rpi/scripts/install_deps.sh"
    echo "Asegurate de haber generado el proyecto con ./elm_327_rpi_oled_v16.sh"
    exit 1
fi
