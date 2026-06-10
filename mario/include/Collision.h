#ifndef COLLISION_H
#define COLLISION_H

#include "../include/Sprite.h"

namespace Collision {
    // Comprueba si dos sprites colisionan en sus posiciones dadas
    bool pixelPerfect(const Sprite& a, int ax, int ay,
                      const Sprite& b, int bx, int by);
}

#endif

