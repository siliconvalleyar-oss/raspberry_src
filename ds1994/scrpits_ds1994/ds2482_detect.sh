#!/bin/bash

# Script: detect_ds1994.sh
# Purpose: Detect DS1994-F5 device on DS2482 (I2C address 0x18)
# Author: Generated for Raspberry Pi

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
I2C_BUS=1
DS2482_ADDR=0x18
WIRE_MASTER_DIR="/sys/bus/w1/devices"
LOG_FILE="/var/log/ds1994_detection.log"

# Function: Log messages with timestamp
log_message() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

# Function: Check if running as root
check_root() {
    if [[ $EUID -ne 0 ]]; then
        echo -e "${RED}This script must be run as root!${NC}"
        echo "Use: sudo $0"
        exit 1
    fi
}

# Function: Enable I2C on Raspberry Pi
enable_i2c() {
    log_message "Checking if I2C is enabled..."
    if ! grep -q "^dtparam=i2c_arm=on" /boot/config.txt; then
        log_message "Enabling I2C in /boot/config.txt..."
        echo "dtparam=i2c_arm=on" >> /boot/config.txt
        echo "I2C enabled. Please reboot and run the script again."
        exit 0
    fi
    
    # Load I2C kernel module if not already loaded
    if ! lsmod | grep -q "^i2c_dev"; then
        modprobe i2c-dev
        log_message "Loaded i2c-dev module"
    fi
}

# Function: Check if I2C tools are installed
check_i2c_tools() {
    log_message "Checking for I2C tools..."
    if ! command -v i2cdetect &> /dev/null; then
        log_message "I2C tools not found. Installing..."
        apt-get update
        apt-get install -y i2c-tools
    fi
}

# Function: Verify DS2482 is present on I2C bus
verify_ds2482() {
    log_message "Verifying DS2482 at address $DS2482_ADDR on I2C bus $I2C_BUS..."
    
    if i2cdetect -y $I2C_BUS | grep -q "$(printf '%02x' $DS2482_ADDR)"; then
        echo -e "${GREEN}✓ DS2482 found at address $DS2482_ADDR${NC}"
        log_message "DS2482 detected at address $DS2482_ADDR"
        return 0
    else
        echo -e "${RED}✗ DS2482 NOT found at address $DS2482_ADDR${NC}"
        log_message "ERROR: DS2482 not detected at address $DS2482_ADDR"
        echo "Please check:"
        echo "  1. Wiring connections (SDA=GPIO2, SCL=GPIO3)"
        echo "  2. Pull-up resistors (4.7kΩ on SDA and SCL)"
        echo "  3. DS2482 power supply"
        return 1
    fi
}

# Function: Register DS2482 with kernel driver
register_ds2482() {
    log_message "Registering DS2482 with kernel driver..."
    
    # Check if already registered
    if [ -d "/sys/bus/i2c/devices/i2c-$I2C_BUS/$I2C_BUS-$(printf '%04x' $DS2482_ADDR)" ]; then
        log_message "DS2482 already registered"
        echo -e "${GREEN}✓ DS2482 already registered${NC}"
        return 0
    fi
    
    # Load the ds2482 kernel module
    if ! lsmod | grep -q "^ds2482"; then
        modprobe ds2482
        log_message "Loaded ds2482 kernel module"
        sleep 1
    fi
    
    # Register the device
    echo "ds2482 $DS2482_ADDR" > /sys/bus/i2c/devices/i2c-$I2C_BUS/new_device 2>/dev/null
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ DS2482 registered successfully${NC}"
        log_message "DS2482 registration successful"
    else
        echo -e "${RED}✗ Failed to register DS2482${NC}"
        log_message "ERROR: Failed to register DS2482"
        return 1
    fi
    
    # Wait for device to be ready
    sleep 2
}

# Function: Scan for 1-Wire devices
scan_1wire_devices() {
    log_message "Scanning for 1-Wire devices..."
    
    # The DS2482 creates a w1_bus_master interface
    # Find the w1 master device
    local w1_master=$(ls /sys/bus/w1/devices/w1_bus_master* 2>/dev/null | head -1)
    
    if [ -z "$w1_master" ]; then
        echo -e "${RED}✗ No 1-Wire master found${NC}"
        log_message "ERROR: No 1-Wire master interface found"
        return 1
    fi
    
    # Trigger a search for 1-Wire devices
    echo "search" > "$w1_master/search" 2>/dev/null
    
    # Wait for search to complete
    sleep 1
    
    # List all detected 1-Wire devices
    local devices=$(ls /sys/bus/w1/devices/ 2>/dev/null | grep -E '^[0-9a-f]{2}-' || echo "")
    
    if [ -z "$devices" ]; then
        echo -e "${YELLOW}! No 1-Wire devices found${NC}"
        log_message "No 1-Wire devices detected"
        return 1
    fi
    
    echo -e "${GREEN}✓ 1-Wire devices found:${NC}"
    echo "$devices"
    
    # Check for DS1994 specifically
    # DS1994 family code is 0x04 (or 0x24 depending on variant)
    # The device ID format is: family_code-serial_number
    
    local ds1994_found=false
    
    for device in $devices; do
        # Family code is the first two characters before the dash
        local family_code=$(echo "$device" | cut -d'-' -f1)
        
        # DS1994-F5 family code is typically 0x04 or 0x24
        if [ "$family_code" = "04" ] || [ "$family_code" = "24" ]; then
            ds1994_found=true
            local serial=$(echo "$device" | cut -d'-' -f2)
            echo -e "${GREEN}  >>> DS1994-F5 DETECTED!${NC}"
            echo -e "      Family Code: $family_code"
            echo -e "      Serial Number: $serial"
            echo -e "      Full ROM ID: $device"
            log_message "DS1994-F5 detected: $device"
            
            # Try to read device information if available
            local device_dir="/sys/bus/w1/devices/$device"
            if [ -f "$device_dir/id" ]; then
                local rom_id=$(cat "$device_dir/id" 2>/dev/null)
                echo -e "      ROM ID: $rom_id"
            fi
            
            # For DS1994, you might need to read the specific w1_slave file
            if [ -f "$device_dir/w1_slave" ]; then
                echo -e "\n      Device Data:"
                cat "$device_dir/w1_slave" | sed 's/^/      /'
            fi
        fi
    done
    
    if [ "$ds1994_found" = false ]; then
        echo -e "\n${YELLOW}! DS1994-F5 not found in detected devices${NC}"
        echo -e "Detected family codes:"
        for device in $devices; do
            echo "  - $(echo $device | cut -d'-' -f1)"
        done
        log_message "DS1994-F5 not detected"
        return 1
    fi
    
    return 0
}

# Function: Read DS1994-F5 specific data (if kernel driver supports)
read_ds1994_data() {
    log_message "Attempting to read DS1994-F5 data..."
    
    local ds1994_devices=$(ls /sys/bus/w1/devices/ 2>/dev/null | grep -E '^(04|24)-')
    
    for device in $ds1994_devices; do
        local device_dir="/sys/bus/w1/devices/$device"
        
        echo -e "\n${GREEN}=== Reading DS1994-F5: $device ===${NC}"
        
        # Check for temperature/clock data
        if [ -f "$device_dir/w1_slave" ]; then
            echo -e "${YELLOW}Raw device data:${NC}"
            cat "$device_dir/w1_slave"
        else
            echo -e "${YELLOW}Note: Full kernel driver support for DS1994 may not be loaded.${NC}"
            echo -e "The device is detected, but you may need to implement custom read/write operations."
            echo -e "\nTo communicate with DS1994-F5 manually:"
            echo -e "1. Use the 'owfs' package for higher-level 1-Wire filesystem support"
            echo -e "2. Or use 'ds9495' tools for direct device access"
        fi
        
        # Display ROM ID in standard format
        echo -e "\n${GREEN}Device Information:${NC}"
        echo -e "  - Type: DS1994-F5 (1-Wire RTC + SRAM)"
        echo -e "  - ROM ID: $device"
        echo -e "  - Interface: 1-Wire via DS2482"
    done
}

# Function: Provide diagnostic information
diagnostic_info() {
    echo -e "\n${YELLOW}=== Diagnostic Information ===${NC}"
    
    # Check kernel modules
    echo -e "\nLoaded kernel modules:"
    lsmod | grep -E "(ds2482|wire|w1)" || echo "  No 1-Wire modules loaded"
    
    # Check I2C devices
    echo -e "\nI2C devices on bus $I2C_BUS:"
    i2cdetect -y $I2C_BUS 2>/dev/null || echo "  Unable to scan I2C bus"
    
    # Check 1-Wire master
    echo -e "\n1-Wire master interface:"
    ls -la /sys/bus/w1/devices/w1_bus_master* 2>/dev/null || echo "  No w1_bus_master found"
    
    # List all 1-Wire devices
    echo -e "\nAll 1-Wire devices:"
    ls /sys/bus/w1/devices/ 2>/dev/null | grep -v "w1_bus_master" || echo "  No devices"
}

# Function: Setup auto-load on boot
setup_autoload() {
    log_message "Setting up auto-load on boot..."
    
    # Add to /etc/modules
    if ! grep -q "^ds2482" /etc/modules; then
        echo "ds2482" >> /etc/modules
        log_message "Added ds2482 to /etc/modules"
    fi
    
    # Add registration to rc.local
    if ! grep -q "ds2482 $DS2482_ADDR" /etc/rc.local; then
        # Insert before the 'exit 0' line
        sed -i "/^exit 0/i echo \"ds2482 $DS2482_ADDR\" > /sys/bus/i2c/devices/i2c-$I2C_BUS/new_device 2>/dev/null" /etc/rc.local
        log_message "Added DS2482 registration to /etc/rc.local"
        echo -e "${GREEN}✓ Auto-load configured${NC}"
    fi
}

# Main execution
main() {
    echo -e "${GREEN}================================${NC}"
    echo -e "${GREEN}DS1994-F5 Detection Script${NC}"
    echo -e "${GREEN}================================${NC}\n"
    
    check_root
    enable_i2c
    check_i2c_tools
    
    # Verify hardware is present
    if ! verify_ds2482; then
        diagnostic_info
        exit 1
    fi
    
    # Register the DS2482
    if ! register_ds2482; then
        diagnostic_info
        exit 1
    fi
    
    # Scan for 1-Wire devices
    if scan_1wire_devices; then
        read_ds1994_data
        echo -e "\n${GREEN}✓ Detection complete!${NC}"
        
        # Ask about auto-load setup
        echo -e "\nDo you want to set up auto-load on boot? (y/n)"
        read -r response
        if [[ "$response" =~ ^[Yy]$ ]]; then
            setup_autoload
        fi
    else
        echo -e "\n${YELLOW}=== Troubleshooting Tips ===${NC}"
        echo -e "1. Check DS1994-F5 wiring:"
        echo -e "   - DS1994-F5 Data pin → DS2482 1-Wire I/O pin"
        echo -e "   - Add 4.7kΩ pull-up resistor on 1-Wire bus"
        echo -e "   - Ensure proper ground connection"
        echo -e "\n2. Try manual 1-Wire search:"
        echo -e "   echo search > /sys/bus/w1/devices/w1_bus_master1/search"
        echo -e "\n3. Check power to the DS1994-F5 (parasitic power from 1-Wire bus)"
        diagnostic_info
        exit 1
    fi
}

# Run main function
main "$@"
