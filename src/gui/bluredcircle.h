#ifndef BLUREDCIRCLE_H
#define BLUREDCIRCLE_H

#include <SFML/Graphics.hpp>

class BluredCircle : public sf::Drawable
{
private:
    const int noCircles;
    const float maxRadius;
    sf::CircleShape** circles;
    static sf::Color color;
    void adjustCircles();
public:
    BluredCircle(float radius, int fragmentation);
    ~BluredCircle();
    void setPosition(const sf::Vector2f& pos);
    void setPosition(float xPos, float yPos) {setPosition(sf::Vector2f(xPos, yPos));}
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif // BLUREDCIRCLE_H
