#ifndef BUTTON_H
#define BUTTON_H

#include "roundedrectangle.h"

enum class ButtonState {NONE, MARKED, CLICKED};

class Button
{
private:
    RoundedRectangle background;
    sf::Sprite content;
    sf::Color colorDef;
    sf::Color colorOnCursor;
    sf::Color colorOnClick;
    bool isMouseAbove = false;
    bool isClicked = false;
    void centraliseContent();
    void scaleContent(sf::Texture* contentTexture);
public:
    Button(float size, sf::Texture* contentTexture);
    void setPosition(const sf::Vector2f& pos);
    void setPosition(float xPos, float yPos) {setPosition(sf::Vector2f(xPos, yPos));}
    void setContentTexture(sf::Texture* contentTexture);
    ButtonState update(sf::RenderWindow* window);
    void render(sf::RenderTarget& target);
};

#endif // BUTTON_H
