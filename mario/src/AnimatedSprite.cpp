#include "../include/AnimatedSprite.h"

AnimatedSprite::AnimatedSprite() : currentTime(0), currentFrame(0), loop(true), playing(true) {}

void AnimatedSprite::addFrame(const Sprite& sprite, float duration) {
    frames.push_back({sprite, duration});
}

void AnimatedSprite::update(float dt) {
    if (!playing || frames.empty()) return;
    currentTime += dt;
    while (currentTime >= frames[currentFrame].duration) {
        currentTime -= frames[currentFrame].duration;
        currentFrame++;
        if (currentFrame >= (int)frames.size()) {
            if (loop) currentFrame = 0;
            else {
                currentFrame = frames.size() - 1;
                playing = false;
            }
        }
    }
}

void AnimatedSprite::reset() {
    currentTime = 0;
    currentFrame = 0;
    playing = true;
}

const Sprite& AnimatedSprite::getCurrentSprite() const {
    return frames[currentFrame].sprite;
}

