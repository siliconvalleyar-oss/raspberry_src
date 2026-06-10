#include "../include/Sprite.h"
#include "../include/picopng.h"
#include <fstream>
#include <vector>

Sprite::Sprite() : w(0), h(0) {}

Sprite::Sprite(const std::string& filename) {
    load(filename);
}

bool Sprite::load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    std::vector<unsigned char> image;
    unsigned long dw, dh;
    int error = decodePNG(image, dw, dh, buffer.data(), buffer.size());
    if (error != 0) return false;
    
    w = dw; h = dh;
    pixels.resize(w * h);
    solid.resize(w * h);
    
    for (size_t i = 0; i < pixels.size(); ++i) {
        size_t idx = i * 4;
        uint8_t r = image[idx];
        uint8_t g = image[idx+1];
        uint8_t b = image[idx+2];
        uint8_t a = image[idx+3];
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
        solid[i] = (a > 0);
    }
    return true;
}

bool Sprite::isSolid(int x, int y) const {
    if (x < 0 || x >= w || y < 0 || y >= h) return false;
    return solid[y * w + x];
}
