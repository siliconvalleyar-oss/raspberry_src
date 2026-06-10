#!/bin/bash
gcc -o test_led_pwm_bcm2835 test_led_pwm_bcm2835.c -lbcm2835
sudo ./test_led_pwm_bcm2835
