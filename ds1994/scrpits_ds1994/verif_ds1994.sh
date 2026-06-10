#!/bin/bash

# Script para verificar el estado completo del DS1994
# Según DATASHEET: READ_MEMORY NO requiere SKIP_ROM
# Dispositivo: 04-00000065968c

#DEVICE_ID="04-00000065968c"
DEVICE_ID="04-000000593dda"
DEVICE_PATH="/sys/devices/w1_bus_master1/${DEVICE_ID}/rw"

echo "=========================================="
echo "   DS1994 DIAGNÓSTICO COMPLETO (CORREGIDO)"
echo "   Dispositivo: ${DEVICE_ID}"
echo "   Protocolo: Sin SKIP_ROM (datasheet)"
echo "=========================================="
echo ""

# Verificar que el dispositivo existe
if [ ! -f "$DEVICE_PATH" ]; then
    echo "❌ ERROR: Dispositivo no encontrado en ${DEVICE_PATH}"
    exit 1
fi

# ============================================================================
# 1. LEER REGISTRO DE ESTADO (0x0200)
# ============================================================================
echo "1. LEYENDO STATUS REGISTER (0x0200)..."
echo ""

# CORRECCIÓN: READ_MEMORY sin SKIP_ROM
# Formato: READ_MEMORY [0xF0] + TA1 + TA2
echo -ne "\xF0\x00\x02" > "$DEVICE_PATH" 2>/dev/null
sleep 0.05

# Leer 1 byte
STATUS=$(dd if="$DEVICE_PATH" bs=1 count=1 2>/dev/null | xxd -p | tr -d '\n')
if [ -z "$STATUS" ]; then
    echo "   ⚠️  No se pudo leer Status Register"
else
    STATUS_VAL=$((0x${STATUS}))
    echo "   Valor: 0x${STATUS} (${STATUS_VAL})"
    echo ""
    echo "   Bits del Status Register:"
    echo "   ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐"
    echo "   │ b7  │ b6  │ b5  │ b4  │ b3  │ b2  │ b1  │ b0  │"
    echo "   ├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤"
    printf "   │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │\n" \
           $((($STATUS_VAL >> 7) & 1)) \
           $((($STATUS_VAL >> 6) & 1)) \
           $((($STATUS_VAL >> 5) & 1)) \
           $((($STATUS_VAL >> 4) & 1)) \
           $((($STATUS_VAL >> 3) & 1)) \
           $((($STATUS_VAL >> 2) & 1)) \
           $((($STATUS_VAL >> 1) & 1)) \
           $((($STATUS_VAL >> 0) & 1))
    echo "   └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘"
    echo ""
    echo "   Significado:"
    [ $((($STATUS_VAL >> 5) & 1)) -eq 1 ] && echo "   • b5 (CCE) - Cycle Counter Interrupt ENABLED"
    [ $((($STATUS_VAL >> 4) & 1)) -eq 1 ] && echo "   • b4 (ITE) - Interval Timer Interrupt ENABLED"  
    [ $((($STATUS_VAL >> 3) & 1)) -eq 1 ] && echo "   • b3 (RTE) - RTC Interrupt ENABLED"
    [ $((($STATUS_VAL >> 2) & 1)) -eq 1 ] && echo "   • b2 (CCF) - Cycle Counter Alarm Flag ACTIVE"
    [ $((($STATUS_VAL >> 1) & 1)) -eq 1 ] && echo "   • b1 (ITF) - Interval Timer Alarm Flag ACTIVE"
    [ $((($STATUS_VAL >> 0) & 1)) -eq 1 ] && echo "   • b0 (RTF) - RTC Alarm Flag ACTIVE"
fi

echo ""
echo "=========================================="
echo ""

# ============================================================================
# 2. LEER REGISTRO DE CONTROL (0x0201)
# ============================================================================
echo "2. LEYENDO CONTROL REGISTER (0x0201)..."
echo ""

# CORRECCIÓN: READ_MEMORY sin SKIP_ROM
echo -ne "\xF0\x01\x02" > "$DEVICE_PATH" 2>/dev/null
sleep 0.05

CONTROL=$(dd if="$DEVICE_PATH" bs=1 count=1 2>/dev/null | xxd -p | tr -d '\n')
if [ -z "$CONTROL" ]; then
    echo "   ⚠️  No se pudo leer Control Register"
else
    CONTROL_VAL=$((0x${CONTROL}))
    echo "   Valor: 0x${CONTROL} (${CONTROL_VAL})"
    echo ""
    echo "   Bits del Control Register:"
    echo "   ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐"
    echo "   │ b7  │ b6  │ b5  │ b4  │ b3  │ b2  │ b1  │ b0  │"
    echo "   ├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤"
    printf "   │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │\n" \
           $((($CONTROL_VAL >> 7) & 1)) \
           $((($CONTROL_VAL >> 6) & 1)) \
           $((($CONTROL_VAL >> 5) & 1)) \
           $((($CONTROL_VAL >> 4) & 1)) \
           $((($CONTROL_VAL >> 3) & 1)) \
           $((($CONTROL_VAL >> 2) & 1)) \
           $((($CONTROL_VAL >> 1) & 1)) \
           $((($CONTROL_VAL >> 0) & 1))
    echo "   └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘"
    echo ""
    echo "   Significado:"
    echo "   ┌─────────────────────────────────────────────────────────────┐"
    
    # b7 - DSEL
    if [ $((($CONTROL_VAL >> 7) & 1)) -eq 1 ]; then
        echo "   │ • DSEL=1 → Delay 123ms                                     │"
    else
        echo "   │ • DSEL=0 → Delay 3.5ms                                     │"
    fi
    
    # b6 - STOP/START
    if [ $((($CONTROL_VAL >> 6) & 1)) -eq 1 ]; then
        echo "   │ • STOP/START=1 → Timer DETENIDO (en modo manual)            │"
    else
        echo "   │ • STOP/START=0 → Timer EJECUTÁNDOSE (en modo manual)        │"
    fi
    
    # b5 - AUTO/MAN
    if [ $((($CONTROL_VAL >> 5) & 1)) -eq 1 ]; then
        echo "   │ • AUTO/MAN=1 → Modo AUTOMÁTICO                              │"
    else
        echo "   │ • AUTO/MAN=0 → Modo MANUAL                                  │"
    fi
    
    # b4 - OSC
    if [ $((($CONTROL_VAL >> 4) & 1)) -eq 1 ]; then
        echo "   │ • OSC=1 → Oscilador ENCENDIDO                               │"
    else
        echo "   │ • OSC=0 → Oscilador APAGADO                                 │"
    fi
    
    # b3 - RO (READ ONLY) - ⚠️ CRÍTICO
    if [ $((($CONTROL_VAL >> 3) & 1)) -eq 1 ]; then
        echo "   │ • RO=1 → MODO SOLO LECTURA ⚠️ PERMANENTE ⚠️                  │"
    else
        echo "   │ • RO=0 → Escritura permitida                                │"
    fi
    
    # b2 - WPC
    if [ $((($CONTROL_VAL >> 2) & 1)) -eq 1 ]; then
        echo "   │ • WPC=1 → Cycle Counter WRITE PROTECTED ⚠️ PERMANENTE ⚠️     │"
    else
        echo "   │ • WPC=0 → Cycle Counter escribible                          │"
    fi
    
    # b1 - WPI
    if [ $((($CONTROL_VAL >> 1) & 1)) -eq 1 ]; then
        echo "   │ • WPI=1 → Interval Timer WRITE PROTECTED ⚠️ PERMANENTE ⚠️    │"
    else
        echo "   │ • WPI=0 → Interval Timer escribible                         │"
    fi
    
    # b0 - WPR
    if [ $((($CONTROL_VAL >> 0) & 1)) -eq 1 ]; then
        echo "   │ • WPR=1 → RTC WRITE PROTECTED ⚠️ PERMANENTE ⚠️               │"
    else
        echo "   │ • WPR=0 → RTC escribible                                    │"
    fi
    
    echo "   └─────────────────────────────────────────────────────────────┘"
fi

echo ""
echo "=========================================="
echo ""

# ============================================================================
# 3. VERIFICAR SI SE PUEDE ESCRIBIR (PRUEBA REAL)
# ============================================================================
echo "3. PRUEBA REAL DE ESCRITURA..."
echo ""

# CORRECCIÓN: READ_MEMORY sin SKIP_ROM
echo -ne "\xF0\x00\x00" > "$DEVICE_PATH" 2>/dev/null
sleep 0.05
ORIGINAL=$(dd if="$DEVICE_PATH" bs=1 count=1 2>/dev/null | xxd -p | tr -d '\n')
echo "   Valor original en 0x0000: 0x${ORIGINAL}"

# CORRECCIÓN: WRITE_SCRATCHPAD sin SKIP_ROM
echo -ne "\x0F\x00\x00\xAA" > "$DEVICE_PATH" 2>/dev/null
sleep 0.05

# CORRECCIÓN: COPY_SCRATCHPAD sin SKIP_ROM
echo -ne "\x55\x00\x00\x00" > "$DEVICE_PATH" 2>/dev/null
sleep 0.05

# Leer después de intentar escribir (READ_MEMORY sin SKIP_ROM)
echo -ne "\xF0\x00\x00" > "$DEVICE_PATH" 2>/dev/null
sleep 0.05
AFTER=$(dd if="$DEVICE_PATH" bs=1 count=1 2>/dev/null | xxd -p | tr -d '\n')
echo "   Valor después de intentar escribir 0xAA: 0x${AFTER}"

if [ "$ORIGINAL" == "$AFTER" ]; then
    echo ""
    echo "   ❌ RESULTADO: No se pudo escribir (el valor no cambió)"
    echo "   ⚠️  Esto confirma que WRITE PROTECT está activo"
else
    echo ""
    echo "   ✅ RESULTADO: Se pudo escribir correctamente"
fi

echo ""
echo "=========================================="
echo ""

# ============================================================================
# 4. DIAGNÓSTICO FINAL
# ============================================================================
echo "4. DIAGNÓSTICO FINAL"
echo ""

if [ ! -z "$CONTROL" ]; then
    CONTROL_VAL=$((0x${CONTROL}))
    
    if [ $CONTROL_VAL -eq 0 ]; then
        echo "   ✅ DISPOSITIVO TOTALMENTE DESBLOQUEADO"
        echo "   → Se puede escribir sin restricciones"
    elif [ $((($CONTROL_VAL >> 3) & 1)) -eq 1 ]; then
        echo "   🔴 DISPOSITIVO EN MODO SOLO LECTURA (RO=1)"
        echo "   → NO se puede escribir en SRAM"
        echo "   → NO se puede modificar RTC/Timer/Counter"
        echo "   → Los bits de write protect son PERMANENTES (OTP)"
    elif [ $CONTROL_VAL -ne 0 ]; then
        echo "   🟡 DISPOSITIVO PARCIALMENTE PROTEGIDO"
        if [ $((($CONTROL_VAL >> 0) & 1)) -eq 1 ]; then
            echo "   → WPR=1: RTC protegido permanentemente"
        fi
        if [ $((($CONTROL_VAL >> 1) & 1)) -eq 1 ]; then
            echo "   → WPI=1: Interval Timer protegido permanentemente"
        fi
        if [ $((($CONTROL_VAL >> 2) & 1)) -eq 1 ]; then
            echo "   → WPC=1: Cycle Counter protegido permanentemente"
        fi
    fi
fi

echo ""
echo "=========================================="
echo "   FIN DEL DIAGNÓSTICO"
echo "=========================================="
