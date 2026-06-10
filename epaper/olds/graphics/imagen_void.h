#pragma once 

#define MAX 5624



namespace EPAPER{
    //unsigned char buffer1[MAX];    
    //unsigned char buffer2[MAX];

    #if(SCREEN==266)
        //#include <graphics/imagen_void.cpp>
       // #define BW_monoBuf (uint8_t *) & buffer1
       // #define BW_0x00Buf (uint8_t *) & buffer2
    #endif
    void func_void ();
}