#pragma once
#include <iostream>
#include <cstring>
#include <qrencode.h>


#define SCALE 5	//4

namespace QR
{

// Definición del tamaño de la imagen
    const int IMAGE_WIDTH = 152;
    const int IMAGE_HEIGHT = 296;
    const int BYTES_PER_ROW { IMAGE_WIDTH / 8};  // 152 bits / 8 = 19 bytes por fila

        struct Qr_gen_t
            {
                Qr_gen_t()=default;
                ~Qr_gen_t()=default;
            public:
                void    setPixel        (int x, int y, bool isBlack) ;
                void    drawQRCode      (const char* data, int scaleFactor);
                int     qr_generator    ();

                private:

                    // Buffer de imagen en blanco y negro
                    unsigned char imageBuffer[BYTES_PER_ROW * IMAGE_HEIGHT] = {0};

            };

}

