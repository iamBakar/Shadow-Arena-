#pragma once
#include "Animation.hpp"

class AnimatedSprite {
public:
    void play(const Animation& anim, bool restart = false);
    void update(float dt);

    void setPosition(float x, float y) { sprite.setPosition(x, y); }
    void move(float dx, float dy)       { sprite.move(dx, dy); }
    sf::Vector2f getPosition() const    { return sprite.getPosition(); }
    void setScale(float sx, float sy)   { sprite.setScale(sx, sy); }
    void setColor(const sf::Color& c)   { sprite.setColor(c); }
    void setFlipped(bool flipped);

    void setOriginY(float oy) { customOriginY = oy; applyFrame(); }

    bool isFinished() const   { return finished; }
    int  getFrameIndex() const { return frameIndex; }

    sf::Sprite& getSprite()               { return sprite; }
    const sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }

private:
    sf::Sprite sprite;
    const Animation* current = nullptr;
    int   frameIndex = 0;
    float elapsed    = 0.f;
    bool  finished   = false;
    bool  flipped    = false;
    float customOriginY = -1.f; // -1 = use abs(frame height)

    void applyFrame();
};
