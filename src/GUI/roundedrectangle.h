#ifndef ROUNDEDRECTANGLE_H
#define ROUNDEDRECTANGLE_H

#include <SFML/Graphics.hpp>

class RoundedRectangle : public sf::Shape
{
private:
    float radius;
    float round;
    static constexpr int pointsCount = 200;
    sf::Vector2f getRefPoint(std::size_t index) const;
public:
    RoundedRectangle(float diameter, float roundProp = 0.4f);
    float getRadius() const {return radius;}
    std::size_t getPointCount() const override {return pointsCount;}
    sf::Vector2f getPoint(std::size_t index) const override;
};

#endif // ROUNDEDRECTANGLE_H
