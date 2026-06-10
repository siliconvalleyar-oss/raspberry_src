#include "../include/Collision.h"
#include <algorithm>

bool Collision::pixelPerfect(const Sprite& a, int ax, int ay,
                             const Sprite& b, int bx, int by) {
    // Rectángulos de intersección
    int left   = std::max(ax, bx);
    int right  = std::min(ax + a.w, bx + b.w);
    int top    = std::max(ay, by);
    int bottom = std::min(ay + a.h, by + b.h);
    if (left >= right || top >= bottom) return false;
    
    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            if (a.isSolid(x - ax, y - ay) && b.isSolid(x - bx, y - by))
                return true;
        }
    }
    return false;
}


