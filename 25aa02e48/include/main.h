#pragma once


#define READ 0b00000011 //Read data from memory array beginning at selected address
#define WRITE 0b00000010 //Write data to memory array beginning at selected address
#define WRDI 0b00000100 //Reset the write enable latch (disable write operations)
#define WREN 0b00000110 //Set the write enable latch (enable write operations)
#define RDSR 0b00000101 //Read STATUS register
#define WRSR 0b00000001 //Write STATUS register
