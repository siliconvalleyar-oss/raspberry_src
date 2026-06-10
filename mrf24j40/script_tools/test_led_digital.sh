#!/bin/bash
gcc -o test_led_digital test_led_digital.c -lbcm2835
sudo ./test_led_digital
