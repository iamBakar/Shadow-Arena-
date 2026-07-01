#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>
#include "Character.hpp"

enum class Scene { Menu, DifficultySelect, Select, Fight, Over };

enum class Difficulty {
    Easy,
    Medium,
    Hard
};

struct MovementInput {
    bool p1Left = false, p1Right = false;
    bool p2Left = false, p2Right = false;
};

class Game {
public:
    Game();

    void update(float dt, const MovementInput& input = {});
    void render(sf::RenderTarget& target); // dispatches to the scene below

    void renderMenu(sf::RenderTarget& target);
    void renderSelect(sf::RenderTarget& target);
    void renderFight(sf::RenderTarget& target);
    void renderOver(sf::RenderTarget& target);

    void handleKeyPressed(sf::Keyboard::Key key);
    void startFight(int p1Index, int p2Index); // also usable directly by tests/preview

    Scene scene = Scene::Menu;
    bool wantsExit = false;

    void renderDifficulty(sf::RenderTarget& target);
private:
    sf::Font font;
    sf::Texture menuBgTex, fightBgTex;
    sf::Sprite menuBgSprite, fightBgSprite;

    std::vector<CharacterDef> roster;
    std::vector<sf::Texture> sheetTextures;

    int selectIndex = 0;
    int p1Choice = 0, p2Choice = 1;

    bool twoPlayerMode = false;      // false = vs AI, true = local 2-player
    bool pickingPlayer2 = false;     // mid-way through the two-stage select
    int player1Pick = 0;             // holds P1's pick while P2 is still choosing

    std::unique_ptr<Character> player1, player2;
    bool playerWon = false;
    float koTimer = -1.f;
    float totalTime = 0.f;

    float enemyThinkTimer = 1.0f;
    float aiMoveDir = 0.f;

    std::mt19937 rng{std::random_device{}()};

    void loadAssets();
    void enemyAI(float dt);
    void resolveHits();
    void handleMovement(float dt, const MovementInput& input);

    void drawCenteredText(sf::RenderTarget& target, const std::string& text,
                           float x, float y, unsigned size, sf::Color color,
                           sf::Color outline = sf::Color::Black, float outlineThickness = 0.f);
    void drawHealthBar(sf::RenderTarget& target, Character& c, float x, float y, bool anchorRight);

    Difficulty difficulty = Difficulty::Medium;
    int difficultyIndex = 1;

    float shakeTime = 0.f;
    float shakeStrength = 0.f;

    bool showBlockSpark = false;
    float aiBlockTimer = 0.f;
    float sparkTimer = 0.f;
    sf::Vector2f sparkPos;

};
