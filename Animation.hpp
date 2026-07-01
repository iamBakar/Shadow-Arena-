#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Animation {
public:
    Animation() = default;

    Animation(sf::Texture* tex, int frameW, int frameH, int rowIndex,
              int numFrames, float frameTime, bool loop)
        : texture(tex), frameTime(frameTime), loop(loop) {
        for (int i = 0; i < numFrames; ++i)
            frames.emplace_back(i * frameW, rowIndex * frameH, frameW, frameH);
    }

    sf::Texture* texture = nullptr;
    std::vector<sf::IntRect> frames;
    float frameTime = 0.1f;
    bool loop = true;
};
