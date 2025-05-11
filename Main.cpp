#include <array>
#include <chrono>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp> // <-- Audio header
#include <iostream>

#include "Headers/Global.hpp"
#include "Headers/DrawText.hpp"
#include "Headers/Pacman.hpp"
#include "Headers/Ghost.hpp"
#include "Headers/GhostManager.hpp"
#include "Headers/ConvertSketch.hpp"
#include "Headers/DrawMap.hpp"
#include "Headers/MapCollision.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode(CELL_SIZE * MAP_WIDTH * SCREEN_RESIZE, (FONT_HEIGHT + CELL_SIZE * MAP_HEIGHT) * SCREEN_RESIZE), "Pac-Man", sf::Style::Close);
    window.setView(sf::View(sf::FloatRect(0, 0, CELL_SIZE * MAP_WIDTH, FONT_HEIGHT + CELL_SIZE * MAP_HEIGHT)));

    // Load background image
    sf::Texture menuTexture;
    if (!menuTexture.loadFromFile("Resources/menu_bg.png")) {
        std::cerr << "Failed to load menu background image!" << std::endl;
    }
    sf::Sprite menuSprite(menuTexture);
    sf::Vector2u textureSize = menuTexture.getSize();
    menuSprite.setOrigin(textureSize.x / 2.0f, textureSize.y / 2.0f);
    sf::Vector2u windowSize = window.getSize();
    menuSprite.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("Resources/arial.ttf")) {
        std::cerr << "Font loading failed! Make sure Resources/arial.ttf exists." << std::endl;
        return -1;
    }

    // Setup start and quit text
    sf::Text startText("Start", font, 40);
    startText.setStyle(sf::Text::Bold);
    startText.setFillColor(sf::Color(139, 0, 0));
    sf::FloatRect startBounds = startText.getLocalBounds();
    startText.setOrigin(startBounds.width / 2, startBounds.height / 2);
    startText.setPosition((CELL_SIZE * MAP_WIDTH) / 2, 300);

    sf::Text quitText("Quit", font, 40);
    quitText.setStyle(sf::Text::Bold);
    quitText.setFillColor(sf::Color(0, 0, 139));
    sf::FloatRect quitBounds = quitText.getLocalBounds();
    quitText.setOrigin(quitBounds.width / 2, quitBounds.height / 2);
    quitText.setPosition((CELL_SIZE * MAP_WIDTH) / 2, 380);

    // Load and play menu music
    sf::Music menuMusic;
    if (!menuMusic.openFromFile("Resources/menu.wav")) {
        std::cerr << "Failed to load menu music!" << std::endl;
    } else {
        menuMusic.setLoop(true);
        menuMusic.play();
    }

    bool showMenu = true;
    window.setView(window.getDefaultView());

    while (showMenu && window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f mousePosF = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (startText.getGlobalBounds().contains(mousePosF)) {
                    showMenu = false;
                } else if (quitText.getGlobalBounds().contains(mousePosF)) {
                    window.close();
                }
            }
        }

        // Hover effects
        sf::Vector2f mousePosF = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        startText.setCharacterSize(startText.getGlobalBounds().contains(mousePosF) ? 48 : 40);
        quitText.setCharacterSize(quitText.getGlobalBounds().contains(mousePosF) ? 48 : 40);

        startBounds = startText.getLocalBounds();
        startText.setOrigin(startBounds.width / 2, startBounds.height / 2);
        startText.setPosition((CELL_SIZE * MAP_WIDTH) / 2, 300);

        quitBounds = quitText.getLocalBounds();
        quitText.setOrigin(quitBounds.width / 2, quitBounds.height / 2);
        quitText.setPosition((CELL_SIZE * MAP_WIDTH) / 2, 380);

        window.clear();
        window.draw(menuSprite);
        window.draw(startText);
        window.draw(quitText);
        window.display();
    }

    if (!window.isOpen())
        return 0;

    menuMusic.stop(); // Stop music when game starts
    window.setView(sf::View(sf::FloatRect(0, 0, CELL_SIZE * MAP_WIDTH, FONT_HEIGHT + CELL_SIZE * MAP_HEIGHT)));

    // === Game starts ===
    bool game_won = false;
    unsigned lag = 0;
    unsigned char level = 0;
    std::chrono::time_point<std::chrono::steady_clock> previous_time;

    std::array<std::string, MAP_HEIGHT> map_sketch = {
        " ################### ",
        " #........#........# ",
        " #o##.###.#.###.##o# ",
        " #.................# ",
        " #.##.#.#####.#.##.# ",
        " #....#...#...#....# ",
        " ####.### # ###.#### ",
        "    #.#   0   #.#    ",
        "#####.# ##=## #.#####",
        "     .  #123#  .     ",
        "#####.# ##### #.#####",
        "    #.#       #.#    ",
        " ####.# ##### #.#### ",
        " #........#........# ",
        " #.##.###.#.###.##.# ",
        " #o.#.....P.....#.o# ",
        " ##.#.#.#####.#.#.## ",
        " #....#...#...#....# ",
        " #.######.#.######.# ",
        " #.................# ",
        " ################### "
    };

    std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH> map{};
    std::array<Position, 4> ghost_positions;

    GhostManager ghost_manager;
    Pacman pacman;

    srand(static_cast<unsigned>(time(0)));
    map = convert_sketch(map_sketch, ghost_positions, pacman);
    ghost_manager.reset(level, ghost_positions);
    previous_time = std::chrono::steady_clock::now();

    while (window.isOpen())
    {
        unsigned delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time).count();
        lag += delta_time;
        previous_time += std::chrono::microseconds(delta_time);

        while (FRAME_DURATION <= lag)
        {
            lag -= FRAME_DURATION;

            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            if (!game_won && !pacman.get_dead())
            {
                game_won = true;

                pacman.update(level, map);
                ghost_manager.update(level, map, pacman);

                for (const auto& column : map)
                {
                    for (const Cell& cell : column)
                    {
                        if (cell == Cell::Pellet)
                        {
                            game_won = false;
                            break;
                        }
                    }
                    if (!game_won) break;
                }

                if (game_won)
                {
                    pacman.set_animation_timer(0);
                }
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
            {
                game_won = false;
                level = pacman.get_dead() ? 0 : level + 1;

                map = convert_sketch(map_sketch, ghost_positions, pacman);
                ghost_manager.reset(level, ghost_positions);
                pacman.reset();
            }

            if (FRAME_DURATION > lag)
            {
                window.clear();

                if (!game_won && !pacman.get_dead())
                {
                    draw_map(map, window);
                    ghost_manager.draw(GHOST_FLASH_START >= pacman.get_energizer_timer(), window);
                    draw_text(0, 0, CELL_SIZE * MAP_HEIGHT, "Level: " + std::to_string(1 + level), window);
                }

                pacman.draw(game_won, window);

                if (pacman.get_animation_over())
                {
                    if (game_won)
                        draw_text(1, 0, 0, "Next level!", window);
                    else
                        draw_text(1, 0, 0, "Game over", window);
                }

                window.display();
            }
        }
    }

    return 0;
}
