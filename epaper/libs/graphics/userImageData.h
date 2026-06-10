//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   userImageData.h
//          License             :   GNU 
//          Author              :   Lio
//          Change History      :
//          Processor           :   ARM
//          Hardware            :		
//          Complier            :   ARM
//          Company             :
//          Dependencies        :
//          Description         :   User set of Image Data
//          brief               :	
//
//////////////////////////////////////////////////////////////////////////////



#pragma once

//2.13"
#if(SCREEN==213) 
#include <graphics/image_213_212x104_BW.cpp>
#include <graphics/image_213_212x104_BWR.cpp>
#define BW_monoBuffer        (uint8_t *) & image_213_212x104_BW_mono
#define BW_0x00Buffer        (uint8_t *) & image_213_212x104_BW_0x00
#define BWR_blackBuffer      (uint8_t *) & image_213_212x104_BWR_blackBuffer
#define BWR_redBuffer        (uint8_t *) & image_213_212x104_BWR_redBuffer
#elif(SCREEN==266)
#include <graphics/image_266_296x152_BW.cpp>
#include <graphics/image_266_296x152_BWR.cpp>
#include <graphics/qr_code.cpp>
#define BW_monoBuffer        (uint8_t *) & image_266_296x152_BW_mono
#define BW_QrBuffer        (uint8_t *) & image_213_212x104_qr
#define BW_0x00Buffer        (uint8_t *) & image_266_296x152_BW_0x00
#define BWR_blackBuffer      (uint8_t *) & image_266_296x152_BWR_blackBuffer
#define BWR_redBuffer        (uint8_t *) & image_266_296x152_BWR_redBuffer
#endif

