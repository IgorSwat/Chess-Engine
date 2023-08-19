#include "src/core/chessboard.h"

int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Chess", sf::Style::Close | sf::Style::Titlebar, settings);
    window.setFramerateLimit(60);
    sf::Event event;
    BoardTextures::loadTextures();
    Chessboard board(&window);
    sf::Cursor cursor;
    if (cursor.loadFromSystem(sf::Cursor::Hand))
        window.setMouseCursor(cursor);
    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        board.updateBoard();
        window.clear(sf::Color(245, 220, 152, 0.5));
        board.render(window);
        window.display();
    }
    return 0;
}
