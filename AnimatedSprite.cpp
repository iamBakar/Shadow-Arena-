#include "AnimatedSprite.hpp"
#include <cstdlib>

void AnimatedSprite::play(const Animation& anim, bool restart) {
    if (current == &anim && !restart) return;
    current = &anim;
    sprite.setTexture(*anim.texture, true);
    frameIndex = 0;
    elapsed = 0.f;
    finished = false;
    applyFrame();
}

void AnimatedSprite::update(float dt) {
    if (!current || current->frames.empty()) return;
    if (finished) return; 

    elapsed += dt;
    while (elapsed >= current->frameTime) {
        elapsed -= current->frameTime;
        frameIndex++;
        if (frameIndex >= static_cast<int>(current->frames.size())) {
            if (current->loop) {
                frameIndex = 0;
            } else {
                frameIndex = static_cast<int>(current->frames.size()) - 1;
                finished = true;
                break;
            }
        }
    }
    applyFrame();
}

void AnimatedSprite::setFlipped(bool f) {
    if (flipped == f) return;
    flipped = f;
    applyFrame();
}

void AnimatedSprite::applyFrame() {
    if (!current || current->frames.empty()) return;
    sf::IntRect rect = current->frames[frameIndex];
    if (flipped) {
        rect.left += rect.width;
        rect.width = -rect.width;
    }
    sprite.setTextureRect(rect);
    float oy = (customOriginY >= 0.f) ? customOriginY
                                       : static_cast<float>(std::abs(rect.height));
    sprite.setOrigin(std::abs(rect.width) / 2.f, oy);
}
