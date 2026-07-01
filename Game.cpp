#include "Game.hpp"
#include "SheetLayout.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <sstream>
#include <algorithm>

namespace {
constexpr float WIN_W = 1280.f, WIN_H = 720.f;

void requireLoad(bool ok, const std::string& what) {
    if (!ok) throw std::runtime_error("Failed to load asset: " + what +
        "\n(Run the executable from the folder that contains the 'assets' directory.)");
}
}

Game::Game() {
    loadAssets();
}

void Game::loadAssets() {
    requireLoad(font.loadFromFile("assets/fonts/font.ttf"), "assets/fonts/font.ttf");
    requireLoad(menuBgTex.loadFromFile("assets/backgrounds/menu_bg.png"), "assets/backgrounds/menu_bg.png");
    requireLoad(fightBgTex.loadFromFile("assets/backgrounds/fight_bg.png"), "assets/backgrounds/fight_bg.png");
    menuBgSprite.setTexture(menuBgTex);
    fightBgSprite.setTexture(fightBgTex);

    roster = {
        { "Martial Hero",  "assets/characters/MartialHero2.png",
          200, 200, 11, {4,4,4,4,3,7,8},   110, 15, 20, 40, 5.7f, 129.f },
        { "Hero Knight",   "assets/characters/HeroKnight.png",
          180, 180, 11, {11,7,7,7,4,11,8}, 150, 20, 18, 50, 5.7f, 115.f },
        { "Evil Wizard",   "assets/characters/EvilWizard2.png",
          250, 250, 11, {8,8,8,8,3,7,8},   90,  18, 18, 55, 3.1f, 167.f },
        { "Medieval King", "assets/characters/MedievalKing2.png",
          160, 111, 11, {8,4,4,4,4,6,8},   130, 16, 25, 45, 5.9f, 105.f },
        { "Fire Mage",     "assets/characters/EvilWizard1.png",
          150, 150, 11, {8,8,8,8,4,5,8},   95,  20, 20, 52, 5.5f, 102.f },
        { "Huntress",      "assets/characters/Huntress1.png",
          150, 150, 11, {8,5,5,7,3,8,8},   105, 14, 22, 45, 7.6f,  97.f },
        { "Shadow Blade",  "assets/characters/MartialHero3.png",
          126, 126, 11, {10,7,6,9,3,11,8}, 115, 18, 16, 48, 7.8f,  82.f },
        { "Huntress II",   "assets/characters/Huntress2.png",
          100, 100, 11, {10,6,6,6,3,10,8}, 100, 16, 16, 50, 8.9f,  67.f },
    };

    sheetTextures.resize(roster.size());
    for (size_t i = 0; i < roster.size(); ++i) {
        requireLoad(sheetTextures[i].loadFromFile(roster[i].sheetFile), roster[i].sheetFile);
        // Use nearest-neighbour filtering to keep pixel art crisp when scaled up
        sheetTextures[i].setSmooth(false);
    }
}

void Game::startFight(int p1Index, int p2Index) {
    p1Choice = p1Index;
    p2Choice = p2Index;
    player1 = std::make_unique<Character>(roster[p1Choice], sheetTextures[p1Choice]);
    player2 = std::make_unique<Character>(roster[p2Choice], sheetTextures[p2Choice]);
    player1->setPosition(WIN_W / 2.f - 150.f, 630.f);
    player2->setPosition(WIN_W / 2.f + 150.f, 630.f);

    player1->setScale(roster[p1Choice].displayScale);
    player2->setScale(roster[p2Choice].displayScale);
    player1->setFacingRight(true);
    player2->setFacingRight(false);
    koTimer = -1.f;
    enemyThinkTimer = 1.0f;
    pickingPlayer2 = false;
    scene = Scene::Fight;
}

void Game::handleKeyPressed(sf::Keyboard::Key key) {
    switch (scene) {
        case Scene::Menu:
            if (key == sf::Keyboard::Enter) {
                scene = Scene::DifficultySelect;
                twoPlayerMode = false;
                pickingPlayer2 = false;
            } else if (key == sf::Keyboard::Tab) {
                twoPlayerMode = true;
                pickingPlayer2 = false;
                scene = Scene::Select;
                selectIndex = 0;
            } else if (key == sf::Keyboard::Escape) {
                wantsExit = true;
            }
            break;

        case Scene::Select: {
            int n = static_cast<int>(roster.size());
            if (key == sf::Keyboard::Left || key == sf::Keyboard::A)
                selectIndex = (selectIndex + n - 1) % n;
            else if (key == sf::Keyboard::Right || key == sf::Keyboard::D)
                selectIndex = (selectIndex + 1) % n;
            else if (key == sf::Keyboard::Enter) {
                if (!twoPlayerMode) {
                    int enemy = selectIndex;
                    if (n > 1) {
                        std::uniform_int_distribution<int> dist(0, n - 1);
                        do { enemy = dist(rng); } while (enemy == selectIndex);
                    }
                    startFight(selectIndex, enemy);
                } else if (!pickingPlayer2) {
                    player1Pick = selectIndex;
                    pickingPlayer2 = true; // now Player 2 picks, same controls/screen
                } else {
                    startFight(player1Pick, selectIndex); // mirror matches allowed
                }
            } else if (key == sf::Keyboard::Escape) {
                if (twoPlayerMode && pickingPlayer2) pickingPlayer2 = false;
                else scene = Scene::Menu;
            }
            break;
        }

        case Scene::Fight:
            if (!player1 || !player1->isAlive() || koTimer >= 0.f) {
                if (key == sf::Keyboard::Escape) scene = Scene::Menu;
                break;
            }
            if (key == sf::Keyboard::A) player1->doPunch();
            else if (key == sf::Keyboard::S) player1->doKick();
            else if (key == sf::Keyboard::D) player1->doRage();
            else if (twoPlayerMode && key == sf::Keyboard::U) player2->doPunch();
            else if (twoPlayerMode && key == sf::Keyboard::I) player2->doKick();
            else if (twoPlayerMode && key == sf::Keyboard::O) player2->doRage();
            else if (key == sf::Keyboard::Escape) scene = Scene::Menu;
            break;

        case Scene::Over:
            if (key == sf::Keyboard::Enter) scene = Scene::Menu;
            else if (key == sf::Keyboard::Escape) wantsExit = true;
            break;

        case Scene::DifficultySelect:
        {
            int maxIndex = 2;

            if (key == sf::Keyboard::Up)
                difficultyIndex = (difficultyIndex + maxIndex) % (maxIndex + 1);
            else if (key == sf::Keyboard::Down)
                difficultyIndex = (difficultyIndex + 1) % (maxIndex + 1);

            else if (key == sf::Keyboard::Enter) {
                if (difficultyIndex == 0) difficulty = Difficulty::Easy;
                else if (difficultyIndex == 1) difficulty = Difficulty::Medium;
                else difficulty = Difficulty::Hard;

                scene = Scene::Select;
                selectIndex = 0;
            }
            else if (key == sf::Keyboard::Escape) {
                scene = Scene::Menu;
            }

            break;
        }
    }
}

void Game::handleMovement(float dt, const MovementInput& input) {
    if (!player1 || !player2) return;

    constexpr float speed = 220.f;
    constexpr float minX = 180.f, maxX = WIN_W - 180.f;
    constexpr float minGap = 200.f;

    float p1dx = 0.f;

    if (input.p1Left)  p1dx -= speed * dt;
    if (input.p1Right) p1dx += speed * dt;

    float p2dx = 0.f;

    if (twoPlayerMode) {
        if (input.p2Left)  p2dx -= speed * dt;
        if (input.p2Right) p2dx += speed * dt;
    }
    else {
        p2dx = aiMoveDir * speed * dt;
    }

    player1->setWalking(p1dx != 0.f);
    player2->setWalking(p2dx != 0.f);

    float oldX1 = player1->getPosition().x;
    float oldX2 = player2->getPosition().x;

    float x1 = std::clamp(oldX1 + p1dx, minX, maxX);
    float x2 = std::clamp(oldX2 + p2dx, minX, maxX);

    x1 = std::min(x1, oldX2 - minGap);
    x2 = std::max(x2, oldX1 + minGap);
    x1 = std::clamp(x1, minX, maxX);
    x2 = std::clamp(x2, minX, maxX);

    player1->moveBy(x1 - oldX1);
    player2->moveBy(x2 - oldX2);
}
void Game::enemyAI(float dt) {
    aiMoveDir = 0.f;
    if (koTimer >= 0.f || scene != Scene::Fight) {
        aiMoveDir = 0.f;
        return;
    }

    if (!player1->isAlive() || !player2->isAlive()) {
        aiMoveDir = 0.f;
        return;
    }

    if (player2->isAttacking())
        aiMoveDir = 0.f;

    float dist = player2->getPosition().x - player1->getPosition().x;
    float absDist = std::abs(dist);

    if (absDist > 260.f) {
        aiMoveDir = (dist > 0.f) ? -1.f : 1.f; // move towards player
    }
    else if (absDist < 180.f) {
        aiMoveDir = (dist > 0.f) ? 1.f : -1.f; // step back slightly
    }
    else {
        aiMoveDir = 0.f; // perfect range
    }

    if (difficulty == Difficulty::Hard) {
        if (player1->isAttacking()) {
            player2->setBlocking(true);

            if (std::rand() % 100 < 30) {
                player2->setBlocking(false);
                player2->doPunch();
            }
            return;
        }

        player2->setBlocking(false);

        enemyThinkTimer -= dt;

        if (enemyThinkTimer <= 0.f) {
            enemyThinkTimer = 0.25f; // extremely fast reaction

            int attack = std::rand() % 3;

            if (attack == 0)
                player2->doPunch();
            else if (attack == 1)
                player2->doKick();
            else if (player2->canUseRage())
                player2->doRage();
        }
    }

    else if (difficulty == Difficulty::Medium) {

        enemyThinkTimer -= dt;

        if (enemyThinkTimer <= 0.f) {
            enemyThinkTimer = 0.6f;

            if (std::rand() % 2 == 0)
                player2->doPunch();
            else
                player2->doKick();
        }
    }

    else {

        enemyThinkTimer -= dt;

        if (enemyThinkTimer <= 0.f) {
            enemyThinkTimer = 1.3f;

            if (std::rand() % 2 == 0)
                player2->doPunch();
            else
                player2->doKick();
        }
    }
}
//void Game::enemyAI(float dt) {
//    if (player2->isBusy()) {
//        aiMoveDir = 0.f;
//    }
//
//    if (player2->isBusy()) { aiMoveDir = 0.f; return; }
//
//    if (aiBlockTimer > 0.f) {
//        aiBlockTimer -= dt;
//        player2->setBlocking(true);
//    }
//    else {
//        player2->setBlocking(false);
//    }
//
//    // AI BLOCKING
//    
//    if (player2->canUseRage()) {
//        aiMoveDir = 0.f;
//        player2->doRage();
//        return;
//    }
//
//    float dist = player2->getPosition().x - player1->getPosition().x;
//
//    if (player1->getState() == FighterState::Punch ||
//        player1->getState() == FighterState::Kick ||
//        player1->getState() == FighterState::Rage) {
//
//        if (difficulty != Difficulty::Easy) {
//            aiBlockTimer = 0.4f;
//            player2->setBlocking(true);
//            return;  // STOP HERE
//        }
//    }
//
//    if (player1->getState() == FighterState::Punch ||
//        player1->getState() == FighterState::Kick ||
//        player1->getState() == FighterState::Rage) {
//
//        if (difficulty != Difficulty::Easy) {
//            aiMoveDir = (dist > 0.f) ? 1.f : -1.f; // run away
//            return;
//        }
//    }
//
//    float preferredRange = (difficulty == Difficulty::Hard) ? 180.f : 280.f;
//    if (std::abs(dist) > preferredRange) {
//        aiMoveDir = (dist > 0.f) ? -1.f : 1.f; // close the distance before deciding to attack
//        return;
//    }
//    aiMoveDir = 0.f;
//
//    enemyThinkTimer -= dt;
//
//    float minDelay, maxDelay;
//    int aggression;
//
//    switch (difficulty) {
//    case Difficulty::Easy:
//        minDelay = 1.3f;
//        maxDelay = 2.2f;
//        aggression = 40;
//        break;
//
//    case Difficulty::Medium:
//        minDelay = 0.6f;
//        maxDelay = 1.1f;
//        aggression = 70;
//        break;
//
//    case Difficulty::Hard:
//        minDelay = 0.03f;   // VERY FAST
//        maxDelay = 0.2f;
//        aggression = 100;
//        break;
//    }
//
//    if (enemyThinkTimer <= 0.f) {
//
//        std::uniform_real_distribution<float> waitDist(minDelay, maxDelay);
//        enemyThinkTimer = waitDist(rng);
//
//        std::uniform_int_distribution<int> roll(0, 100);
//        int r = roll(rng);   // get random number ONCE
//
//        if (r < aggression) {
//
//            if (difficulty == Difficulty::Hard && player2->canUseRage())
//                player2->doRage();
//
//            else if (r % 2 == 0)
//                player2->doPunch();
//
//            else
//                player2->doKick();
//        }
//    }
//}

void Game::resolveHits() {
    constexpr float attackRange = 310.f;
    bool inRange = std::abs(player1->getPosition().x - player2->getPosition().x) <= attackRange;

    // PLAYER 1 HITS PLAYER 2
    if (player1->consumeHitEvent()) {
        int dmg = 0;
        switch (player1->getState()) {
        case FighterState::Punch: dmg = player1->getPunchDamage(); break;
        case FighterState::Kick:  dmg = player1->getKickDamage();  break;
        case FighterState::Rage:  dmg = player1->getRageDamage();  break;
        default: break;
        }

        if (dmg > 0 && inRange) {
            // CHECK BLOCK
            if (player2->isBlocking()) {
                showBlockSpark = true;
                sparkTimer = 0.1f;
                sparkPos = player2->getPosition();
            }

            player2->takeDamage(dmg);

            shakeTime = 0.15f;
            shakeStrength = 8.f;
        }
    }

    // PLAYER 2 HITS PLAYER 1
    if (player2->isAlive() && player2->consumeHitEvent()) {
        int dmg = 0;
        switch (player2->getState()) {
        case FighterState::Punch: dmg = player2->getPunchDamage(); break;
        case FighterState::Kick:  dmg = player2->getKickDamage();  break;
        case FighterState::Rage:  dmg = player2->getRageDamage();  break;
        default: break;
        }

        if (dmg > 0 && inRange) {

            if (player1->isBlocking()) {
                showBlockSpark = true;
                sparkTimer = 0.1f;
                sparkPos = player1->getPosition();
            }

            player1->takeDamage(dmg);

            shakeTime = 0.15f;
            shakeStrength = 8.f;
        }
    }
}
void Game::update(float dt, const MovementInput& input) {
    totalTime += dt;
    if (scene != Scene::Fight || !player1 || !player2) return;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        player1->setBlocking(true);
    else
        player1->setBlocking(false);

    if (twoPlayerMode) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))
            player2->setBlocking(true);
        else
            player2->setBlocking(false);
    }

    player1->update(dt);
    player2->update(dt);

    if (koTimer < 0.f) {
        handleMovement(dt, input);
        if (!twoPlayerMode) enemyAI(dt);
        resolveHits();
        if (!player1->isAlive() || !player2->isAlive()) {
            playerWon = player1->isAlive();
            koTimer = 1.4f;
        }
    } else {
        koTimer -= dt;
        if (koTimer <= 0.f) scene = Scene::Over;
    }

    if (sparkTimer > 0.f) {
        sparkTimer -= dt;
        if (sparkTimer <= 0.f)
            showBlockSpark = false;
    }
}

void Game::drawCenteredText(sf::RenderTarget& target, const std::string& text,
                             float x, float y, unsigned size, sf::Color color,
                             sf::Color outline, float outlineThickness) {
    sf::Text t(text, font, size);
    t.setFillColor(color);
    if (outlineThickness > 0.f) {
        t.setOutlineColor(outline);
        t.setOutlineThickness(outlineThickness);
    }
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(x, y);
    target.draw(t);
}

void Game::drawHealthBar(sf::RenderTarget& target, Character& c, float x, float y, bool anchorRight) {
    const float barW = 320.f, barH = 26.f;

    float frac = c.getMaxHealth() > 0 ?
        c.getDisplayHealth() / c.getMaxHealth() : 0.f;

    float drawX = anchorRight ? x - barW : x;

    sf::RectangleShape border(sf::Vector2f(barW + 4.f, barH + 4.f));
    border.setPosition(drawX - 2.f, y - 2.f);
    border.setFillColor(sf::Color(20, 20, 20));
    target.draw(border);

    sf::RectangleShape back(sf::Vector2f(barW, barH));
    back.setPosition(drawX, y);
    back.setFillColor(sf::Color(60, 60, 60));
    target.draw(back);
    float hpPercent = (float)c.getHealth() / c.getMaxHealth();

    sf::Color fillColor;

    if (hpPercent < 0.3f) {
        // blinking effect
        if (((int)(totalTime * 6) % 2) == 0)
            fillColor = sf::Color::Red;
        else
            fillColor = sf::Color::White;
    }
    else if (hpPercent > 0.5f)
        fillColor = sf::Color(70, 220, 90);
    else if (hpPercent > 0.25f)
        fillColor = sf::Color(240, 200, 40);
    else
        fillColor = sf::Color(230, 60, 50);

    sf::RectangleShape fill(sf::Vector2f(barW * frac, barH));
    fill.setFillColor(fillColor);
    if (anchorRight) fill.setPosition(drawX + barW * (1.f - frac), y);
    else fill.setPosition(drawX, y);
    target.draw(fill);

    std::ostringstream hpText;
    hpText << c.getName() << "  " << c.getHealth() << "/" << c.getMaxHealth();
    drawCenteredText(target, hpText.str(), drawX + barW / 2.f, y - 18.f, 18, sf::Color::White);

    if (hpPercent <= 0.3f) {
        drawCenteredText(target, "RAGE READY!",
            drawX + barW / 2.f, y + barH + 16.f,
            18, sf::Color(255, 80, 0));
    }
}

void Game::render(sf::RenderTarget& target) {
    switch (scene) {
        case Scene::Menu:   renderMenu(target);   break;
        case Scene::Select: renderSelect(target); break;
        case Scene::Fight:  renderFight(target);  break;
        case Scene::Over:   renderOver(target);   break;
        case Scene::DifficultySelect: renderDifficulty(target); break;
    }
}

void Game::renderMenu(sf::RenderTarget& target) {
    target.draw(menuBgSprite);
}

void Game::renderSelect(sf::RenderTarget& target) {
    target.draw(fightBgSprite);

    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 130));
    target.draw(overlay);

    std::string title = !twoPlayerMode ? "CHOOSE YOUR FIGHTER"
                       : (pickingPlayer2 ? "PLAYER 2: CHOOSE YOUR FIGHTER" : "PLAYER 1: CHOOSE YOUR FIGHTER");
    drawCenteredText(target, title, WIN_W / 2.f, 40.f, 34,
                      sf::Color(220, 190, 140), sf::Color::Black, 3.f);

    if (twoPlayerMode && pickingPlayer2) {
        drawCenteredText(target, "Player 1 picked: " + roster[player1Pick].name,
                          WIN_W / 2.f, 80.f, 18, sf::Color(190, 220, 255));
    }

    const int n = static_cast<int>(roster.size());
    constexpr int COLS = 4;
    const float slotW = WIN_W / COLS;
    const float groundY[2] = { 290.f, 520.f };

    for (int i = 0; i < n; ++i) {
        int col = i % COLS;
        int row = i / COLS;
        float cx = slotW * col + slotW / 2.f;
        float gy = groundY[row];
        bool selected = (i == selectIndex);

        int fw = roster[i].frameW, fh = roster[i].frameH;
        float baseScale = roster[i].displayScale * 0.42f;
        float scale = selected ? baseScale * 1.12f : baseScale * 0.88f;

        sf::Sprite portrait;
        portrait.setTexture(sheetTextures[i]);
        portrait.setTextureRect(sf::IntRect(0, 0, fw, fh));
        portrait.setScale(scale, scale);
        portrait.setOrigin(fw / 2.f, roster[i].originY);
        portrait.setPosition(cx, gy);
        portrait.setColor(selected ? sf::Color::White : sf::Color(170, 170, 170));
        target.draw(portrait);

        if (selected) {
            float bw = 155.f, bh = 195.f;
            sf::RectangleShape box(sf::Vector2f(bw, bh));
            box.setOrigin(bw / 2.f, bh);
            box.setPosition(cx, gy);
            box.setFillColor(sf::Color(255, 200, 50, 30));
            box.setOutlineColor(sf::Color(220, 190, 140));
            box.setOutlineThickness(3.f);
            target.draw(box);
        }

        drawCenteredText(target, roster[i].name, cx, gy + 14.f, 17,
                          selected ? sf::Color(255, 220, 120) : sf::Color(210, 210, 210));

        std::ostringstream stats;
        stats << "HP " << roster[i].maxHealth;
        drawCenteredText(target, stats.str(), cx, gy + 36.f, 14, sf::Color(180, 180, 180));
    }

    drawCenteredText(target, "<-  ->  Select      ENTER  Confirm      ESC  Back",
                      WIN_W / 2.f, 664.f, 18, sf::Color(255, 255, 255, 220));
}

void Game::renderFight(sf::RenderTarget& target) {
    float offsetX = 0.f;
    float offsetY = 0.f;

    if (shakeTime > 0.f) {
        shakeTime -= 0.016f;

        offsetX = (rand() % 9 - 4);
        offsetY = (rand() % 9 - 4);
    }

    fightBgSprite.setPosition(offsetX, offsetY);
    target.draw(fightBgSprite);

    if (!player1 || !player2) return;
    player1->moveBy(offsetX);
    player2->moveBy(offsetX);

    player1->draw(target);
    player2->draw(target);

    player1->moveBy(-offsetX);
    player2->moveBy(-offsetX);

    drawHealthBar(target, *player1, 40.f, 36.f, false);
    drawHealthBar(target, *player2, WIN_W - 40.f, 36.f, true);

    drawCenteredText(target, "VS", WIN_W / 2.f, 50.f, 28, sf::Color::White, sf::Color::Black, 2.f);

    std::string controls = twoPlayerMode
        ? "P1: <-/-> Move, A Punch, S Kick, D Rage, W Block  |  P2: J/L Move, U Punch, I Kick, O Rage, K Block"
        : "<-/-> Move  A: Punch  S: Kick  D: Rage  W: Block";
    drawCenteredText(target, controls, WIN_W / 2.f, WIN_H - 28.f, twoPlayerMode ? 16u : 18u,
                      sf::Color(255, 255, 255, 215));

    if (koTimer >= 0.f) {
        drawCenteredText(target, "K.O.!", WIN_W / 2.f, WIN_H / 2.f, 84,
                          sf::Color(255, 60, 40), sf::Color::Black, 5.f);
    }

    if (showBlockSpark) {
        sf::RectangleShape spark(sf::Vector2f(30.f, 30.f));
        spark.setFillColor(sf::Color(255, 220, 50, 200));
        spark.setOrigin(15.f, 15.f);
        spark.setPosition(sparkPos);

        target.draw(spark);
    }
}

void Game::renderOver(sf::RenderTarget& target) {
    target.draw(fightBgSprite);
    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 140));
    target.draw(overlay);
    // PANEL UNDER TEXT
    sf::RectangleShape bottomPanel(sf::Vector2f(700.f, 100.f));
    bottomPanel.setOrigin(350.f, 80.f);
    bottomPanel.setPosition(WIN_W / 2.f, 580.f);

    bottomPanel.setFillColor(sf::Color(0, 0, 0, 120));
    bottomPanel.setOutlineColor(sf::Color(255, 215, 80, 120));
    bottomPanel.setOutlineThickness(2.f);

    target.draw(bottomPanel);


    std::string headline = twoPlayerMode
        ? (playerWon ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!")
        : (playerWon ? "YOU WIN!" : "YOU LOSE");
    sf::Color color = playerWon ? sf::Color(220, 190, 140) : sf::Color(230, 60, 60);
    drawCenteredText(target, headline, WIN_W / 2.f, 280.f, 80, color, sf::Color::Black, 4.f);

    std::string sub = playerWon
        ? (roster[p1Choice].name + " defeats " + roster[p2Choice].name + "!")
        : (roster[p2Choice].name + " defeats " + roster[p1Choice].name + "!");
    drawCenteredText(target, sub, WIN_W / 2.f, 360.f, 26, sf::Color::White);

    drawCenteredText(target, "ENTER: Main Menu      ESC: Quit", WIN_W / 2.f, 540.f, 24,
                      sf::Color(255, 255, 255, 220));
}

void Game::renderDifficulty(sf::RenderTarget& target) {
    target.draw(fightBgSprite);

    // dark overlay
    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 140));
    target.draw(overlay);

    sf::RectangleShape panel(sf::Vector2f(400.f, 280.f));
    panel.setOrigin(200.f, 140.f);
    panel.setPosition(WIN_W / 2.f, 380.f);

    panel.setFillColor(sf::Color(0, 0, 0, 100));
    panel.setOutlineColor(sf::Color(255, 215, 80, 80));
    panel.setOutlineThickness(2.f);

    target.draw(panel);

    drawCenteredText(target, "SELECT DIFFICULTY", WIN_W / 2.f, 140.f, 48,
        sf::Color(220, 190, 140), sf::Color::Black, 4.f);

    std::vector<std::string> options = { "EASY", "MEDIUM", "HARD" };

    for (int i = 0; i < 3; i++) {
        float y = 300.f + i * 90.f;

        bool selected = (i == difficultyIndex);

        sf::Color color = selected ? sf::Color(220, 190, 140)
            : sf::Color(200, 200, 200);

        drawCenteredText(target, options[i], WIN_W / 2.f, y, 36,
            color, sf::Color::Black, selected ? 3.f : 0.f);
    }

    drawCenteredText(target,
        "UP / DOWN: Select    ENTER: Confirm    ESC: Back",
        WIN_W / 2.f, 650.f, 20,
        sf::Color(220, 220, 220));
}