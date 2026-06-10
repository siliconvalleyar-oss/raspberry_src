#!/bin/bash

#NAMEFILE="add_tx_led_spet5.sh"
NAMEFILE=$1

touch $NAMEFILE
chmod +x $NAMEFILE
nano $NAMEFILE
./$NAMEFILE
