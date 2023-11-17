#pragma once

#include <SFML/Graphics.hpp>

class BluredCircle : public sf::Drawable
{
public:
    BluredCircle(float radius, int fragmentation);
    ~BluredCircle();

    void setPosition(const sf::Vector2f& pos);
    void setPosition(float xPos, float yPos);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    void adjustCircles();

    sf::CircleShape** circles;
    const int noCircles;
    const float maxRadius;
};