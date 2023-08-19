#include "moveentry.h"

sf::Font MoveEntry::font = sf::Font();

MoveEntry::MoveEntry(const sf::Vector2f& initialPos, float initialSize, const sf::Texture& buttonTexture)
    : leftCorner(initialPos), size(initialSize), button(0.6f * initialSize, buttonTexture)
{
    loadFont();
    moveName.setFont(font);
    evaluation.setFont(font);
    moveName.setString("None");
    moveName.setCharacterSize(static_cast<unsigned int>(0.15f * initialSize));
    moveName.setFillColor(sf::Color::Black);
    evaluation.setString("None");
    evaluation.setCharacterSize(static_cast<unsigned int>(0.15f * initialSize));
    evaluation.setFillColor(sf::Color::Black);
    setAlignment();
}

void MoveEntry::setAlignment()
{
    float textLength = moveName.getGlobalBounds().width;
    moveName.setPosition(leftCorner.x + 0.5f * (size - textLength), leftCorner.y);
    button.setPosition(leftCorner.x + 0.2f * size, leftCorner.y + 0.2f * size);
    evaluation.setPosition(leftCorner.x + 0.5f * (size - textLength), leftCorner.y + 0.85 * size);
}

void MoveEntry::setData(const sf::Texture& texture, const std::string& movename, const std::string& eval)
{
    button.setContentTexture(texture);
    moveName.setString(movename);
    evaluation.setString(eval);
    setAlignment();
}

void MoveEntry::render(sf::RenderTarget& target)
{
    target.draw(moveName);
    button.render(target);
    target.draw(evaluation);
}

