#include "button.h"

Button::Button(float size, const sf::Texture& contentTexture) : background(size)
{
    content.setTexture(contentTexture);
    scaleContent(contentTexture);
    centraliseContent();
    background.setOutlineThickness(0.05f * size);
    background.setOutlineColor(sf::Color::Black);
    colorDef = sf::Color::White;
    colorOnCursor = sf::Color(201, 234, 245);
    colorOnClick = sf::Color(140, 218, 245);
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

void Button::setPosition(const sf::Vector2f& pos)
{
    background.setPosition(pos);
    centraliseContent();
}

void Button::setContentTexture(const sf::Texture& contentTexture)
{
    content.setTexture(contentTexture, true);
    scaleContent(contentTexture);
}

ButtonState Button::update(sf::RenderWindow* window)
{
    sf::FloatRect bound = background.getGlobalBounds();
    sf::Vector2f mousePos(static_cast<float>(sf::Mouse::getPosition(*window).x),
                          static_cast<float>(sf::Mouse::getPosition(*window).y));
    if (!bound.contains(mousePos))
    {
        if (isMouseAbove)
            background.setFillColor(colorDef);
        isMouseAbove = isClicked = false;
        return ButtonState::NONE;
    }
    else
    {
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if (isClicked || !isMouseAbove)
                background.setFillColor(colorOnCursor);
            isMouseAbove = true;
            isClicked = false;
            return ButtonState::MARKED;
        }
        else
        {
            if (!isClicked)
                background.setFillColor(colorOnClick);
            isMouseAbove = isClicked = true;
            return ButtonState::CLICKED;
        }
    }
}

void Button::render(sf::RenderTarget& target)
{
    target.draw(background);
    target.draw(content);
}

