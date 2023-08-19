#pragma once

#include "../logic/piece.h"
#include <SFML/Graphics.hpp>

class PieceImage : public sf::Drawable
{
public:
    PieceImage(const sf::Vector2f& initPos);
    PieceImage(const sf::Texture& texture, const sf::Vector2f& initPos);

    void setTexture(const sf::Texture& texture);
    void setState(bool newState);

    void setTempPosition(const sf::Vector2f& pos);
    void setPermPosition(const sf::Vector2f& pos);
    void resetToStablePosition();
    void updateOrigin(const sf::Vector2f& mousePos);
    void updateOrigin(const sf::Vector2i& mousePos);
    const sf::Vector2f& getStablePosition() const { return stablePosition; }
    const sf::Vector2f& getCurrentPosition() const { return image.getPosition(); }
    bool isActive() const { return state; }
    sf::FloatRect getGlobalBounds() const { return image.getGlobalBounds(); }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    sf::Sprite image;
    sf::Vector2f stablePosition;
    bool state;
};