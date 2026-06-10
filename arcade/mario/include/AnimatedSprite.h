#ifndef ANIMATEDSPRITE_H
#define ANIMATEDSPRITE_H

#include "../include/Sprite.h"
#include <vector>

struct AnimationFrame {
    Sprite sprite;
    float duration; // segundos
};

class AnimatedSprite {
public:
    std::vector<AnimationFrame> frames;
    float currentTime;
    int currentFrame;
    bool loop;
    bool playing;
    
    AnimatedSprite();
    void addFrame(const Sprite& sprite, float duration);
    void update(float dt);
    void reset();
    const Sprite& getCurrentSprite() const;
};

#endif

