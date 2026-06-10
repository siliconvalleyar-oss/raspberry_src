#!/bin/bash

# ============================================================================
# DIAGNÓSTICO Y OPTIMIZACIÓN DEL BUS 1-WIRE PARA MÚLTIPLES DS1994
# ============================================================================

BUS_PATH="/sys/devices/w1_bus_master1"
LOG_FILE="/tmp/w1_debug.log"

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║         OPTIMIZACIÓN BUS 1-WIRE - MÚLTIPLES DISPOSITIVOS           ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

# Verificar que el bus existe
if [ ! -d "$BUS_PATH" ]; then
    echo "❌ ERROR: Bus 1-Wire no encontrado en $BUS_PATH"
    echo "   Ejecute: sudo modprobe w1-gpio && sudo modprobe w1-therm"
    exit 1
fi

# Función para contar dispositivos (CORREGIDA)
count_devices() {
    ls "$BUS_PATH"/ 2>/dev/null | grep -c -E "^[0-9a-f]{2}-" | tr -d '\n' | xargs
}

# Función para obtener lista de dispositivos
get_devices() {
    ls "$BUS_PATH"/ 2>/dev/null | grep -E "^[0-9a-f]{2}-"
}

# ============================================================================
# 1. AUMENTAR TIMEOUTS Y PARÁMETROS DEL BUS
# ============================================================================
echo "🔧 1. CONFIGURANDO PARÁMETROS DEL BUS..."
echo ""

# Aumentar timeout de búsqueda (segundos)
if [ -f "$BUS_PATH/w1_master_timeout" ]; then
    OLD=$(cat "$BUS_PATH/w1_master_timeout" 2>/dev/null || echo "desconocido")
    echo "   ⏱️  Timeout búsqueda: $OLD → 10"
    echo 10 | sudo tee "$BUS_PATH/w1_master_timeout" > /dev/null 2>&1
fi

# Aumentar timeout en microsegundos
if [ -f "$BUS_PATH/w1_master_timeout_us" ]; then
    OLD=$(cat "$BUS_PATH/w1_master_timeout_us" 2>/dev/null || echo "desconocido")
    echo "   ⏱️  Timeout (μs): $OLD → 50000"
    echo 50000 | sudo tee "$BUS_PATH/w1_master_timeout_us" > /dev/null 2>&1
fi

# Configurar pull-up (si está disponible)
if [ -f "$BUS_PATH/w1_master_pullup" ]; then
    OLD=$(cat "$BUS_PATH/w1_master_pullup" 2>/dev/null || echo "desconocido")
    echo "   🔌 Pull-up: $OLD → 1"
    echo 1 | sudo tee "$BUS_PATH/w1_master_pullup" > /dev/null 2>&1
fi

# Configurar reintentos máximos
if [ -f "$BUS_PATH/w1_master_max_slave_count" ]; then
    OLD=$(cat "$BUS_PATH/w1_master_max_slave_count" 2>/dev/null || echo "desconocido")
    echo "   📊 Máx esclavos: $OLD → 20"
    echo 20 | sudo tee "$BUS_PATH/w1_master_max_slave_count" > /dev/null 2>&1
fi

echo ""
echo "✅ Parámetros configurados"
echo ""

# ============================================================================
# 2. DIAGNÓSTICO DEL ESTADO ACTUAL
# ============================================================================
echo "📊 2. DIAGNÓSTICO DEL ESTADO ACTUAL..."
echo ""

echo "   📟 Dispositivos detectados AHORA:"
DEVICES_BEFORE=$(get_devices)
if [ -z "$DEVICES_BEFORE" ]; then
    echo "      (ninguno)"
    COUNT_BEFORE=0
else
    echo "$DEVICES_BEFORE" | while read dev; do
        echo "      ✅ $dev"
    done
    COUNT_BEFORE=$(echo "$DEVICES_BEFORE" | wc -l | tr -d ' ')
fi

echo ""
echo "   📈 Total dispositivos detectados: $COUNT_BEFORE"
echo ""

# ============================================================================
# 3. BÚSQUEDA INTENSIVA DE DISPOSITIVOS
# ============================================================================
echo "🔍 3. BÚSQUEDA INTENSIVA DE DISPOSITIVOS..."
echo ""

for attempt in {1..15}; do
    echo -n "   Intento $attempt/15: "
    
    # Forzar búsqueda
    echo 1 | sudo tee "$BUS_PATH/w1_master_search" > /dev/null 2>&1
    
    # Esperar respuesta del bus
    sleep 2
    
    # Verificar dispositivos encontrados (con conteo corregido)
    DEVICES_NOW=$(get_devices)
    COUNT_NOW=$(echo "$DEVICES_NOW" | grep -c -E "^[0-9a-f]{2}-" 2>/dev/null | tr -d '\n' | xargs)
    
    # Asegurar que COUNT_NOW sea un número
    if [ -z "$COUNT_NOW" ] || [ "$COUNT_NOW" = "" ]; then
        COUNT_NOW=0
    fi
    
    echo "encontrados: $COUNT_NOW dispositivo(s)"
    
    # Mostrar qué dispositivos
    if [ "$COUNT_NOW" -gt 0 ]; then
        echo "$DEVICES_NOW" | while read dev; do
            echo "            📟 $dev"
        done
    fi
    
    # Si encontramos nuevos dispositivos, actualizar
    if [ "$COUNT_NOW" -gt "$COUNT_BEFORE" ] 2>/dev/null; then
        echo "   🎯 Nuevo(s) dispositivo(s) detectado(s)"
        COUNT_BEFORE=$COUNT_NOW
    fi
    
    # Pequeña pausa entre intentos
    sleep 1
done

echo ""
echo "✅ Búsqueda completada"
echo ""

# ============================================================================
# 4. VERIFICACIÓN DE CONEXIÓN POR DISPOSITIVO
# ============================================================================
echo "🔌 4. VERIFICANDO CONEXIÓN DE CADA DISPOSITIVO..."
echo ""

DEVICES=$(get_devices)
if [ -z "$DEVICES" ]; then
    echo "   ❌ No hay dispositivos conectados"
else
    echo "$DEVICES" | while read dev; do
        DEV_PATH="$BUS_PATH/$dev/rw"
        echo -n "   📟 $dev: "
        
        if [ -f "$DEV_PATH" ]; then
            # Intentar leer 1 byte del dispositivo usando READ_ROM
            echo -ne "\x33" | sudo tee "$DEV_PATH" > /dev/null 2>&1
            sleep 0.1
            RESPONSE=$(sudo dd if="$DEV_PATH" bs=1 count=1 2>/dev/null | xxd -p | tr -d '\n')
            
            if [ -n "$RESPONSE" ] && [ "$RESPONSE" != "ff" ] && [ "$RESPONSE" != "FF" ]; then
                echo "✅ RESPONDE (0x$RESPONSE)"
            elif [ "$RESPONSE" == "ff" ] || [ "$RESPONSE" == "FF" ]; then
                echo "⚠️  RESPUESTA 0xFF (puede estar ocupado o protegido)"
            else
                echo "❌ NO RESPONDE"
            fi
        else
            echo "❌ ARCHIVO rw NO ENCONTRADO"
        fi
    done
fi

echo ""

# ============================================================================
# 5. ESTADÍSTICAS DEL BUS
# ============================================================================
echo "📈 5. ESTADÍSTICAS DEL BUS..."
echo ""

if [ -f "$BUS_PATH/w1_master_attempts" ]; then
    echo "   Intentos de búsqueda: $(cat $BUS_PATH/w1_master_attempts 2>/dev/null)"
fi

if [ -f "$BUS_PATH/w1_master_slave_count" ]; then
    echo "   Esclavos detectados: $(cat $BUS_PATH/w1_master_slave_count 2>/dev/null)"
fi

if [ -f "$BUS_PATH/w1_master_slaves" ]; then
    echo "   Lista de esclavos:"
    cat "$BUS_PATH/w1_master_slaves" 2>/dev/null | while read slave; do
        echo "      📟 $slave"
    done
fi

echo ""

# ============================================================================
# 6. RECOMENDACIONES FINALES
# ============================================================================
echo "💡 6. RECOMENDACIONES PARA MEJORAR DETECCIÓN:"
echo ""

FINAL_COUNT=$(get_devices | wc -l | tr -d ' ')
if [ -z "$FINAL_COUNT" ]; then
    FINAL_COUNT=0
fi

if [ "$FINAL_COUNT" -eq 0 ]; then
    echo "   🔴 NINGÚN DISPOSITIVO DETECTADO"
    echo ""
    echo "   Verifique:"
    echo "   1. Resistencia pull-up de 2.2kΩ a 4.7kΩ entre GPIO4 y 3.3V"
    echo "   2. Conexión de GND entre Raspberry Pi y el iButton"
    echo "   3. iButton bien insertado en el socket"
    echo "   4. Limpiar contactos del iButton con alcohol"
    echo "   5. Probar con un solo dispositivo primero"
elif [ "$FINAL_COUNT" -eq 1 ]; then
    echo "   🟡 Solo $FINAL_COUNT dispositivo detectado"
    echo ""
    echo "   Para múltiples dispositivos:"
    echo "   1. Usar resistencia pull-up de 2.2kΩ (más fuerte)"
    echo "   2. Mantener cables cortos (< 50cm)"
    echo "   3. Conectar dispositivos uno por uno"
    echo "   4. Verificar que cada DS1994 funcione individualmente"
else
    echo "   🟢 $FINAL_COUNT dispositivo(s) detectado(s)"
    echo ""
    echo "   Para mejorar estabilidad:"
    echo "   1. Usar cable blindado si las conexiones son largas"
    echo "   2. Agregar resistencia pull-up adicional (2.2kΩ total)"
    echo "   3. Reducir velocidad del bus si hay interferencia"
fi

echo ""
echo "════════════════════════════════════════════════════════════════════"
echo "   DIAGNÓSTICO COMPLETADO - LOG GUARDADO EN: $LOG_FILE"
echo "════════════════════════════════════════════════════════════════════"

# Guardar log
{
    echo "=== DIAGNÓSTICO 1-WIRE $(date) ==="
    echo ""
    echo "Dispositivos encontrados:"
    get_devices
    echo ""
    echo "Parámetros del bus:"
    cat "$BUS_PATH"/w1_master_* 2>/dev/null
    echo ""
    echo "Estado de conexión:"
    for dev in $(get_devices); do
        echo -n "$dev: "
        DEV_PATH="$BUS_PATH/$dev/rw"
        if [ -f "$DEV_PATH" ]; then
            echo -ne "\x33" | sudo tee "$DEV_PATH" > /dev/null 2>&1
            sleep 0.1
            RESPONSE=$(sudo dd if="$DEV_PATH" bs=1 count=1 2>/dev/null | xxd -p | tr -d '\n')
            echo "response=$RESPONSE"
        else
            echo "no_rw_file"
        fi
    done
} > "$LOG_FILE" 2>/dev/null

echo ""
echo "📁 Log guardado en: $LOG_FILE"
echo ""

# Mostrar comandos útiles
echo "════════════════════════════════════════════════════════════════════"
echo "   COMANDOS ÚTILES"
echo "════════════════════════════════════════════════════════════════════"
echo ""
echo "Para ver dispositivos manualmente:"
echo "  ls $BUS_PATH/ | grep '04-'"
echo ""
echo "Para forzar búsqueda manual:"
echo "  echo 1 | sudo tee $BUS_PATH/w1_master_search"
echo ""
echo "Para reiniciar el bus:"
echo "  sudo rmmod w1_gpio w1_therm"
echo "  sudo modprobe w1_gpio"
echo "  sudo modprobe w1_therm"
echo ""

