#!/bin/bash 

#comandos con ds2842

echo -e "\t\t#wire reset"
#wire reset 
result=$(sudo i2cset -y 1 0x18 0xb4 2>&1)

#if [[ ! $result == 'Error: Write failed' ]];then 
if [[ $? -eq 0 ]]; then
	sudo i2cget -y 1 0x18
else
	#sudo i2cget -y 1 0x18
	echo "Error: $result"
fi


#device reset
echo -e "\t\tdevice reset"
sudo i2cset -y 1 0x18 0xf0
sudo i2cget -y 1 0x18

#device  set read pointer
echo -e "\t\tdevice  set read pointer"
sudo i2cset -y 1 0x18 0xe1
sudo i2cget -y 1 0x18



#write configuration
echo -e "\t\twrite configuration"
sudo i2cset -y 1 0x18 0xd2
sudo i2cget -y 1 0x18

echo -e "\t\t1 wire single bit"
#wire reset 
sudo i2cset -y 1 0x18 0x87
sudo i2cget -y 1 0x18



echo -e "\t\t1 wire write byte"
#wire reset 
sudo i2cset -y 1 0x18 0xa5
sudo i2cget -y 1 0x18


echo -e "\t\t1 wire Triplet"
#wire reset 
sudo i2cset -y 1 0x18 0x78
sudo i2cget -y 1 0x18


# Configura la dirección del DS2482 en el bus I2C y el comando para resetear el bus 1-Wire
DS2482_I2C_ADDR=0x18
RESET_CMD=0xF0  # Comando para resetear el bus 1-Wire (F0 es un ejemplo, verifica el comando correcto para tu configuración)

# Envía el comando de reset al DS2482
sudo i2cset -y 1 $DS2482_I2C_ADDR 0xD0  # Enviar el comando para resetear (D0 puede ser el comando de configuración para tu caso)
result=$(sudo i2cset -y 1 $DS2482_I2C_ADDR $RESET_CMD)

# Verifica si el comando fue exitoso
if [[ $? -eq 0 ]]; then
    echo "Reset exitoso. Verificando estado..."
    # Puedes usar i2cget para leer el estado del DS2482 o verificar el bus 1-Wire
    sudo i2cget -y 1 $DS2482_I2C_ADDR
else
    echo "Error: Fallo en el reset del DS2482."
fi
