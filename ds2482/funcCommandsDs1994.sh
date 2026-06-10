#!/bin/bash

# Definición de la función
function run_command {
    local command=$1
    local title=$2

    echo -e "\t\t$title"
    result=$(sudo i2cset -y 1 0x18 $command 2>&1)

    if [[ $? -eq 0 ]]; then
        sudo i2cget -y 1 0x18
    else
        echo -e "\t\t\t\t\t$result"
    fi
sleep 0.1
}

# Llamada a la función con diferentes comandos y títulos
run_command 0xb4 "#wire reset"
run_command 0xf0 "device reset"
run_command 0xe1 "device set read pointer"
run_command 0xd2 "write configuration"
run_command 0x87 "1 wire single bit"
run_command 0xa5 "1 wire write byte"
run_command 0x78 "1 wire Triplet"
run_command 0x96 "1 wire read byte"

#run_command 0xc3 "configuration register"

