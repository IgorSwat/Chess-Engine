#ifndef PROMOTIONBAR_H
#define PROMOTIONBAR_H

#include "pieceimage.h"
#include "textures.h"

enum class PromotionChoice {NONE, KNIGHT, BISHOP, ROOK, QUEEN};

class PromotionBar
{
private:
    static constexpr float tileSize = 100.f;
    static constexpr float scaleFactor = 0.8f;
    static PromotionChoice choices[4];
    sf::Color normal;
    sf::Color onCursor;
    sf::Color onClick;
    sf::Vector2f position;
    sf::CircleShape* circles[4];
    sf::Sprite* sprites[4];
    void setAlignment();
public:
    PromotionBar(Side side);
    ~PromotionBar();
    void setPosition(const sf::Vector2f& pos);
    void setPosition(float xPos, float yPos);
    PromotionChoice update(sf::RenderWindow* window);
    void render(sf::RenderTarget& target);
};

#endif // PROMOTIONBAR_H
