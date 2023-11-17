#include "button.h"

namespace {
    constexpr float OUTLINE_THICKNESS_RATIO = 0.05f;
    const sf::Color colorDefault = sf::Color::White;
    const sf::Color colorOnCover = sf::Color(201, 234, 245);
    const sf::Color colorOnClick = sf::Color(140, 218, 245);
}


Button::Button(ButtonType btype, float size, const sf::Texture& contentTexture) 
    : background(size), buttonType(btype)
{
    background.setOutlineThickness(OUTLINE_THICKNESS_RATIO * size);
    background.setOutlineColor(sf::Color::Black);

    content.setTexture(contentTexture);
    scaleContent(contentTexture);
    centraliseContent();
}

void Button::setPosition(const sf::Vector2f& pos)
{
    background.setPosition(pos);
    centraliseContent();
}

bool Button::updateState()
{
    sf::FloatRect bound = background.getGlobalBounds();
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    if (!bound.contains(sf::Vector2f(mousePos)))
    {
        if (isMouseAbove)
            background.setFillColor(colorDefault);
        isMouseAbove = isClicked = false;
        return false;
    }
    else
    {
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if (isClicked || !isMouseAbove)
                background.setFillColor(colorOnCover);
            isMouseAbove = true;
            isClicked = false;
            return false;
        }
        else
        {
            if (!isClicked)
                background.setFillColor(colorOnClick);
            isMouseAbove = isClicked = true;
            return true;
        }
    }
}

void Button::centraliseContent()
{
    sf::Vector2f backgroundPos = background.getPosition();
    float xPos = backgroundPos.x + background.getRadius() - content.getGlobalBounds().width / 2.f;
    float yPos = backgroundPos.y + background.getRadius() - content.getGlobalBounds().height / 2.f;
    content.setPosition(xPos, yPos);
}

void Button::scaleContent(const sf::Texture& contentTexture)
{
    unsigned int textureSize = std::max(contentTexture.getSize().x, contentTexture.getSize().y);
    float scaleFactor = 1.6f * background.getRadius() / textureSize;
    content.setScale(scaleFactor, scaleFactor);
}