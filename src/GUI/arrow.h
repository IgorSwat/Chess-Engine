#ifndef ARROW_H
#define ARROW_H

#include <SFML/Graphics.hpp>
#include <iostream>

class MoveArrow : public sf::Drawable, public sf::Transformable
{
private:
    sf::VertexArray vertices;
    float length;
    static constexpr float radius = 13.f;
    static constexpr float arrowhead = 65.f;
    static constexpr float gap = 30.f;
    void update();
public:
    MoveArrow(float len);
    void setColor(sf::Color color);
    void setLength(float len);
    float getLength() const {return length;}
    void matchToSquares(const sf::Vector2f& initSquare, const sf::Vector2f& targetSquare);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    static constexpr float getRadius() {return radius;}
    static constexpr float getArrowHead() {return arrowhead;}
    static constexpr float getGap() {return gap;}
};

#endif // ARROW_H
