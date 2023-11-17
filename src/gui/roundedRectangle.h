#pragma once

#include "SFML/Graphics.hpp"

class RoundedRectangle : public sf::Shape
{
public:
	RoundedRectangle(float diameter, float roundProp = 0.4f, std::size_t noPoints = 200);

	float getRadius() const;
	std::size_t getPointCount() const override;
	sf::Vector2f getPoint(std::size_t index) const override;

private:
	sf::Vector2f getRefPoint(std::size_t index) const;

	const float radius;
	const float roundFactor;
	const std::size_t pointsCount;
};



inline float RoundedRectangle::getRadius() const
{
	return radius;
}

inline std::size_t RoundedRectangle::getPointCount() const
{
	return pointsCount;
}