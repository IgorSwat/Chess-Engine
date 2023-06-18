#include "bluredcircle.h"
#include <cmath>

sf::Color BluredCircle::color = sf::Color::Red;

BluredCircle::BluredCircle(float radius, int fragmentation) : noCircles(fragmentation), maxRadius(radius)
{
    circles = new sf::CircleShape* [fragmentation];
    for (int i = 0; i < noCircles; i++) {
        circles[i] = new sf::CircleShape(maxRadius - i * maxRadius / noCircles);
        circles[i]->setFillColor(sf::Color(color.r, color.g, color.b, static_cast<int>(255 * (i + 1) / noCircles)));
    }
    adjustCircles();
}

BluredCircle::~BluredCircle()
{
    for (int i = 0; i < noCircles; i++)
        delete circles[i];
}

void BluredCircle::adjustCircles()
{
    sf::Vector2f origin = circles[0]->getPosition();
    for (int i = 1; i < noCircles; i++)
    {
        float xPos = origin.x + i * maxRadius * sqrt(2.f) / (2.f * noCircles);
        float yPos = origin.y + i * maxRadius * sqrt(2.f) / (2.f * noCircles);
        circles[i]->setPosition(xPos, yPos);
    }
}

void BluredCircle::setPosition(const sf::Vector2f& pos)
{
    circles[0]->setPosition(pos);
    adjustCircles();
}

void BluredCircle::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (int i = 0; i < noCircles; i++)
        target.draw(*circles[i], states);
}

