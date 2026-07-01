#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include "AnimatedSprite.hpp"

enum class FighterState { Idle, Punch, Kick, Rage, Hurt, KO, Walk };
constexpr int kNumStates = 7;

// Per-animation frame timing constants (seconds per frame)
namespace AnimTiming {
    constexpr float IDLE  = 0.14f;
    constexpr float PUNCH = 0.06f;
    constexpr float KICK  = 0.06f;
    constexpr float RAGE  = 0.05f;
    constexpr float HURT  = 0.09f;
    constexpr float KO    = 0.12f;
    constexpr float WALK  = 0.08f;
}

// Per-character definition including variable frame counts from the sprite pack.
struct CharacterDef {
    std::string name;
    std::string sheetFile;
    int frameW, frameH;            // pixel size of one frame
    int maxCols;
    int frameCounts[kNumStates];  
    int maxHealth;
    int punchDmg;
    int kickDmg;
    int rageDmg;
    float displayScale;
    float originY;   
};

class Character {
public:
    Character(const CharacterDef& def, sf::Texture& sheetTexture);

    void update(float dt);
    void draw(sf::RenderTarget& target);

    void doPunch();
    void doKick();
    void doRage();
    void takeDamage(int dmg);

    bool isAlive() const { return health > 0; }
    bool isBusy() const;
    bool canUseRage() const { return rageReady && isAlive() && health <= maxHealth * 0.3f; }

    int  getHealth() const { return health; }
    int  getMaxHealth() const { return maxHealth; }
    const std::string& getName() const { return name; }
    FighterState getState() const { return state; }

    void setPosition(float x, float y);
    void setScale(float s);
    void setFacingRight(bool right);
    void setWalking(bool walking);
    void moveBy(float dx);
    sf::Vector2f getPosition() const;

    int getPunchDamage() const { return punchDmg; }
    int getKickDamage()  const { return kickDmg; }
    int getRageDamage()  const { return rageDmg; }

    bool consumeHitEvent();
    void setBlocking(bool b) { blocking = b; }
    bool isBlocking() const { return blocking; }

    bool isAttacking() const;
    float getDisplayHealth() const;
private:
    std::string name;
    int maxHealth, health;
    int punchDmg, kickDmg, rageDmg;
    bool rageReady = true;

    bool blocking = false;

    FighterState state = FighterState::Idle;
    std::array<Animation, kNumStates> animations;
    AnimatedSprite anim;
    float displayHealth = 0.f;

    bool hitEventArmedThisAction = false;
    bool hitEventFired = false;
    int  hitFrame = 0; 

    void setState(FighterState s, bool restart = true);
};
