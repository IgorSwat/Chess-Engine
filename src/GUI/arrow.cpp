#include "arrow.h"
#include <cmath>
#include <exception>

MoveArrow::MoveArrow(float len) : vertices(sf::TriangleStrip, 10), length(len)
{
    update();
    setColor(sf::Color::Black);
    setOrigin(sf::Vector2f(0.f, radius));
}

void MoveArrow::update()
{
    vertices[0].position = sf::Vector2f(0.f, radius);
    vertices[1].position = sf::Vector2f(radius, 0.f);
    vertices[2].position = sf::Vector2f(radius, 2 * radius);
    vertices[3].position = sf::Vector2f(length + radius, 0.f);
    vertices[4].position = sf::Vector2f(length + radius, 2 * radius);
    vertices[5].position = sf::Vector2f(length + arrowhead + radius, radius);
    vertices[6].position = sf::Vector2f(length + radius, -gap);
    vertices[7].position = sf::Vector2f(length + radius, 2 * radius + gap);
    vertices[8].position = sf::Vector2f(length + arrowhead + radius, radius);
    vertices[9].position = sf::Vector2f(length + radius, 2 * radius);
}

void MoveArrow::setColor(sf::Color color)
{
    for (unsigned int i = 0; i < vertices.getVertexCount(); i++)
        vertices[i].color = color;
}

void MoveArrow::setLength(float len)
{
    if (len < 0)
        throw std::invalid_argument("Invalid negative argument given to Arrow\n");
    length = len;
    update();
}

void MoveArrow::matchToSquares(const sf::Vector2f& initSquare, const sf::Vector2f& targetSquare)
{
    static const float pi = 3.141592654f;
    setPosition(initSquare);
    float distance = sqrt(std::pow(targetSquare.x - initSquare.x, 2) + std::pow(targetSquare.y - initSquare.y, 2));
    setLength(distance - getArrowHead() - getRadius());
    float angle = std::atan2(targetSquare.y - initSquare.y, targetSquare.x - initSquare.x);
    setRotation(angle * 180 / pi);
}

void MoveArrow::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(vertices, states);
}
