#include "roundedRectangle.h"
#include <cmath>

RoundedRectangle::RoundedRectangle(float diameter, float roundProp, std::size_t noPoints)
    : radius(diameter / 2.f), roundFactor(roundProp), pointsCount(noPoints)
{
    update();
}

sf::Vector2f RoundedRectangle::getRefPoint(std::size_t index) const
{
    int part = static_cast<int>(4 * index / pointsCount);
    if (part == 0)
        return sf::Vector2f((2.f - roundFactor) * radius, roundFactor * radius);
    if (part == 1)
        return sf::Vector2f(roundFactor * radius, roundFactor * radius);
    if (part == 2)
        return sf::Vector2f(roundFactor * radius, (2.f - roundFactor) * radius);
    return sf::Vector2f((2.f - roundFactor) * radius, (2.f - roundFactor) * radius);
}

sf::Vector2f RoundedRectangle::getPoint(std::size_t index) const
{
    static const float pi = 3.141592654f;
    const sf::Vector2f refPoint = getRefPoint(index);
    const float tranAngle = 4 * (pi / 2.f) / pointsCount;

    float angle = static_cast<int>(4 * index / pointsCount) * (pi / 2);
    index = index % (pointsCount / 4);
    angle += index * tranAngle;

    float x = std::cos(angle) * roundFactor * radius;
    float y = std::sin(angle) * roundFactor * radius;
    return sf::Vector2f(refPoint.x + x, refPoint.y - y);
}
