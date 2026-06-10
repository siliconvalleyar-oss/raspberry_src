#!/bin/bash

echo "=== RESET BUS 1-WIRE ==="

# 1. Descargar módulos
echo "1. Descargando módulos..."
sudo rmmod w1_gpio w1_therm 2>/dev/null
sleep 2

# 2. Recargar
echo "2. Recargando módulos..."
sudo modprobe w1-gpio
sudo modprobe w1-therm
sleep 2

# 3. Configurar timeouts (si existen)
if [ -f "/sys/devices/w1_bus_master1/w1_master_timeout" ]; then
    echo 10 | sudo tee /sys/devices/w1_bus_master1/w1_master_timeout > /dev/null
fi

if [ -f "/sys/devices/w1_bus_master1/w1_master_timeout_us" ]; then
    echo 50000 | sudo tee /sys/devices/w1_bus_master1/w1_master_timeout_us > /dev/null
fi

# 4. Búsqueda intensiva
echo "3. Buscando dispositivos..."
for i in {1..5}; do
    echo "   Intento $i/5"
    echo 1 | sudo tee /sys/devices/w1_bus_master1/w1_master_search > /dev/null
    sleep 1
done

# 5. Resultados
echo ""
echo "4. Dispositivos detectados:"
ls /sys/devices/w1_bus_master1/ 2>/dev/null | grep "04-" || echo "   (ninguno)"

COUNT=$(ls /sys/devices/w1_bus_master1/ 2>/dev/null | grep -c "04-")
echo ""
echo "Total: $COUNT dispositivo(s)"
echo "=== RESET COMPLETADO ==="

