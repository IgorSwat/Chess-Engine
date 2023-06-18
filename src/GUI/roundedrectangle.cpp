#include "roundedrectangle.h"
#include <cmath>
#include <iostream>

RoundedRectangle::RoundedRectangle(float diameter, float roundProp)
{
    radius = diameter / 2.f;
    round = roundProp;
    update();
}

sf::Vector2f RoundedRectangle::getRefPoint(std::size_t index) const
{
    int part = static_cast<int>(4 * index / pointsCount);
    if (part == 0)
        return sf::Vector2f((2.f - round) * radius, round * radius);
    if (part == 1)
        return sf::Vector2f(round * radius, round * radius);
    if (part == 2)
        return sf::Vector2f(round * radius, (2.f - round) * radius);
    return sf::Vector2f((2.f - round) * radius, (2.f - round) * radius);
}

sf::Vector2f RoundedRectangle::getPoint(std::size_t index) const
{
    static const float pi = 3.141592654f;
    const sf::Vector2f refPoint = getRefPoint(index);
    const float tranAngle = 4 * (pi / 2.f) / pointsCount;

    float angle = static_cast<int>(4 * index / pointsCount) * (pi / 2);
    index = index % (pointsCount / 4);
    angle += index * tranAngle;

    float x = std::cos(angle) * round * radius;
    float y = std::sin(angle) * round * radius;
    return sf::Vector2f(refPoint.x + x, refPoint.y - y);
}

