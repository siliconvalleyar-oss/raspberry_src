
#include <qr/qr_gen.h>
#include <iostream>

namespace QR{        

    // Función para setear un píxel en el buffer (0 = blanco, 1 = negro)
    void Qr_gen_t::setPixel(int x, int y, bool isBlack) {
        if (x < 0 || x >= IMAGE_WIDTH || y < 0 || y >= IMAGE_HEIGHT) return;
        int byteIndex = (y * BYTES_PER_ROW) + (x / 8);
        int bitIndex = 7 - (x % 8);
        if (isBlack) {
            imageBuffer[byteIndex] |= (1 << bitIndex);  // Poner a 1 el bit correspondiente
        } else {
            imageBuffer[byteIndex] &= ~(1 << bitIndex); // Poner a 0 el bit correspondiente
        }
    }

    // Función para generar y dibujar el QR en el buffer con un factor de escala
    void Qr_gen_t::drawQRCode(const char* data, int scaleFactor) {
        QRcode* qrcode = QRcode_encodeString(data, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
        if (!qrcode) {
            std::cerr << "Error al generar el código QR." << std::endl;
            return;
        }
        
        // Tamaño del código QR generado
        int qrSize = qrcode->width;
        
        // Calcular la posición para centrar el QR en el buffer de imagen
        int scaledQrSize = qrSize * scaleFactor;
        int xOffset = (IMAGE_WIDTH - scaledQrSize) / 2;
        int yOffset = (IMAGE_HEIGHT - scaledQrSize) / 2;
        
        // Dibujar el QR en el buffer, escalando los píxeles
        for (int y = 0; y < qrSize; y++) {
            for (int x = 0; x < qrSize; x++) {
                bool isBlack = qrcode->data[y * qrSize + x] & 0x01;  // Píxel blanco o negro
                // Dibujar el píxel escalado
                for (int dy = 0; dy < scaleFactor; ++dy) {
                    for (int dx = 0; dx < scaleFactor; ++dx) {
                        setPixel(x * scaleFactor + dx + xOffset, y * scaleFactor + dy + yOffset, isBlack);
                    }
                }
            }
        }
        
        QRcode_free(qrcode);  // Liberar la memoria del QR
    }



    int Qr_gen_t::qr_generator() {
        // Datos de la red móvil (usuario y contraseña)
        const char* networkData = "WIFI:T:WPA;S:SSID;P:Password;;";
        
        // Limpiar el buffer de imagen (llenarlo de blanco)
        std::memset(imageBuffer, 0x00, sizeof(imageBuffer));

        // Escalar un 20% más que el valor base
        int scaleFactor = SCALE;  // Incremento del 33% en lugar del 20%, ya que los píxeles deben ser enteros

        // Dibujar el código QR en el buffer con el factor de escala
        drawQRCode(networkData, scaleFactor);
        
        // Imprimir el contenido del buffer en formato hexadecimal (opcional para depuración)
        std::cout<<"[[maybe_unused]]unsigned char const image_213_212x104_qr[]= \n"<< std::endl;
        std::cout<< "{\n"<< std::endl;
        for (int i = 0; i < IMAGE_HEIGHT; ++i) {
            for (int j = 0; j < BYTES_PER_ROW; ++j) {
                std::cout << "0x"<<std::hex << (int)imageBuffer[i * BYTES_PER_ROW + j] << ",";
            }
            std::cout<< std::endl;
        }
        std::cout<<" \n};"<< std::endl;

        return 0;
    }



}