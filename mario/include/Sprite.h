#ifndef SPRITE_H
#define SPRITE_H

#include <cstdint>
#include <vector>
#include <string>

struct Sprite {
    std::vector<uint32_t> pixels;  // RGBA
    std::vector<bool> solid;       // true si el píxel no es transparente (alpha > 0)
    int w, h;
    
    Sprite();
    Sprite(const std::string& filename);
    bool load(const std::string& filename);
    
    // Devuelve true si el punto (x,y) local es sólido
    bool isSolid(int x, int y) const;
};

#endif
