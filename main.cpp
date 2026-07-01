#include <SFML/Graphics.hpp>
#include <iostream>
#include "Game.hpp"

int main() {
    sf::RenderWindow window(
        sf::VideoMode::getDesktopMode(),
        "Game",
        sf::Style::Fullscreen
    );

    sf::View view;
    view.setSize(1280.f, 720.f);
    view.setCenter(1280.f / 2.f, 720.f / 2.f);

    view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));

    window.setView(view);

    try {
        Game game;
        sf::Clock clock;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close();
                else if (event.type == sf::Event::KeyPressed) game.handleKeyPressed(event.key.code);
            }

            if (game.wantsExit) window.close();

            MovementInput input;
            input.p1Left  = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
            input.p1Right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
            input.p2Left  = sf::Keyboard::isKeyPressed(sf::Keyboard::J);
            input.p2Right = sf::Keyboard::isKeyPressed(sf::Keyboard::L);

            float dt = clock.restart().asSeconds();
            game.update(dt, input);

            window.clear(sf::Color::Black);
            game.render(window);
            window.display();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
