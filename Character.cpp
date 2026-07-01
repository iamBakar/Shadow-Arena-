#include "Character.hpp"
#include "SheetLayout.hpp"
#include <cstdlib>

Character::Character(const CharacterDef& def, sf::Texture& sheetTexture)
    : name(def.name), maxHealth(def.maxHealth), health(def.maxHealth),
      punchDmg(def.punchDmg), kickDmg(def.kickDmg), rageDmg(def.rageDmg) {

    int fw = def.frameW, fh = def.frameH, mc = def.maxCols;
    const float timings[kNumStates] = {
        AnimTiming::IDLE, AnimTiming::PUNCH, AnimTiming::KICK, AnimTiming::RAGE,
        AnimTiming::HURT, AnimTiming::KO, AnimTiming::WALK
    };
    const bool loops[kNumStates] = { true, false, false, false, false, false, true };

    for (int r = 0; r < kNumStates; ++r) {
        int n = def.frameCounts[r];
        Animation a;
        a.texture   = &sheetTexture;
        a.frameTime = timings[r];
        a.loop      = loops[r];
        for (int c = 0; c < n; ++c)
            a.frames.emplace_back(c * fw, r * fh, fw, fh);
        animations[r] = a;
    }

    anim.play(animations[0]);
  
    if (def.originY > 0.f) anim.setOriginY(def.originY);
    displayHealth = static_cast<float>(health);
}

void Character::setState(FighterState s, bool restart) {
    state = s;
    hitEventArmedThisAction = (s == FighterState::Punch ||
                                s == FighterState::Kick  ||
                                s == FighterState::Rage);
    hitEventFired = false;
    // hit frame = roughly halfway through the animation (feels natural)
    int idx = static_cast<int>(s);
    hitFrame = std::max(0, static_cast<int>(animations[idx].frames.size()) / 2);
    anim.play(animations[idx], restart);
}

bool Character::isBusy() const {
    switch (state) {
        case FighterState::Punch:
        case FighterState::Kick:
        case FighterState::Rage:
        case FighterState::Hurt:
        case FighterState::KO:
            return !anim.isFinished() || state == FighterState::KO;
        default:
            return false;
    }
}

void Character::doPunch() {
    if (!isAlive() || isBusy()) return;
    setState(FighterState::Punch);
}

bool Character::isAttacking() const {
    return state == FighterState::Punch ||
        state == FighterState::Kick ||
        state == FighterState::Rage;
}

void Character::doKick() {
    if (!isAlive() || isBusy()) return;
    setState(FighterState::Kick);
}

void Character::doRage()  {
    if (!isAlive() || isBusy() || !canUseRage()) return;
    setState(FighterState::Rage);
    rageReady = false;
}

void Character::takeDamage(int dmg) {
    if (!isAlive()) return;

    if (blocking) {
        dmg /= 3; // take only 1/3 damage
    }
    health -= dmg;
    if (health < 0) health = 0;

    setState(health == 0 ? FighterState::KO : FighterState::Hurt);
}

bool Character::consumeHitEvent() {
    if (!hitEventArmedThisAction || hitEventFired) return false;
    if (anim.getFrameIndex() >= hitFrame) {
        hitEventFired = true;
        return true;
    }
    return false;
}

void Character::update(float dt) {
    anim.update(dt);

    if (displayHealth > health) {
        displayHealth -= 120.f * dt; // speed of animation
        if (displayHealth < health)
            displayHealth = (float)health;
    }

    if (anim.isFinished() && state != FighterState::KO)
        if (state != FighterState::Idle)
            setState(FighterState::Idle);
}

float Character::getDisplayHealth() const {
    return displayHealth;
}

void Character::draw(sf::RenderTarget& target) {
    if (blocking) {
        anim.getSprite().setColor(sf::Color(150, 150, 150)); // darker gray when blocks 
    }
    else {
        anim.getSprite().setColor(sf::Color(255, 255, 255)); // normal 
    }
    target.draw(anim.getSprite());
}

void Character::setPosition(float x, float y) { anim.setPosition(x, y); }
void Character::setScale(float s) { anim.setScale(s, s); }
void Character::setWalking(bool walking) {
    if (!isAlive() || isBusy()) return;
    if (walking) { if (state != FighterState::Walk) setState(FighterState::Walk); }
    else if (state == FighterState::Walk) setState(FighterState::Idle);
}
void Character::moveBy(float dx) { anim.move(dx, 0.f); }
sf::Vector2f Character::getPosition() const { return anim.getPosition(); }
void Character::setFacingRight(bool right) { anim.setFlipped(!right); }
