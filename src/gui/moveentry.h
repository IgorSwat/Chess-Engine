#ifndef MOVEENTRY_H
#define MOVEENTRY_H

#include "button.h"
#include <string>

class MoveEntry
{
private:
    sf::Vector2f leftCorner;
    float size;
    Button button;
    sf::Text moveName;
    sf::Text evaluation;
    static sf::Font font;
    void setAlignment();
public:
    MoveEntry(const sf::Vector2f& initialPos, float initialSize, const sf::Texture& buttonTexture);
    void setData(const sf::Texture& texture, const std::string& movename, const std::string& eval);
    void setPosition(const sf::Vector2f& pos);
    ButtonState update(sf::RenderWindow* window) {return button.update(window);}
    void render(sf::RenderTarget& target);
    static void loadFont() {font.loadFromFile("resource/Fraset-Display.ttf");}
};

#endif // MOVEENTRY_H
