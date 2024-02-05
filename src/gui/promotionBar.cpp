#include "promotionBar.h"
#include "../logic/types.h"


namespace {
    constexpr float SPRITE_SCALE_FACTOR = 0.8f;
    const sf::Color colorDefault = sf::Color(115, 119, 128);
    const sf::Color colorOnCover = sf::Color(149, 151, 158);
    const sf::Color colorOnClick = sf::Color(190, 202, 232);

    constexpr PieceType PROMOTION_TYPES[PROMOTION_CHOICE_RANGE] = {
        QUEEN, ROOK, BISHOP, KNIGHT
    };
}


PromotionBar::PromotionBar(Color side, float tileSize)
    : entrySize(tileSize)
{
    const float radius = tileSize / 2.f;
    for (int i = 0; i < PROMOTION_CHOICE_RANGE; i++) {
        entries.push_back({ sf::CircleShape(radius), sf::Sprite() });
        entries[i].circle.setFillColor(colorDefault);
        entries[i].sprite.setScale(SPRITE_SCALE_FACTOR, SPRITE_SCALE_FACTOR);

        Piece promotionPiece = make_piece(side, PROMOTION_TYPES[i]);
        entries[i].sprite.setTexture(BoardTextures::pieceTexture(promotionPiece));
    }
}

PromotionChoice PromotionBar::update()
{
    PromotionChoice choice = PromotionChoice::NONE;
    for (int i = 0; i < PROMOTION_CHOICE_RANGE; i++) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
        sf::CircleShape& circle = entries[i].circle;
        if (circle.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                circle.setFillColor(colorOnClick);
                choice = PromotionChoice(i);
            }
            else
                circle.setFillColor(colorOnCover);
        }
        else
            circle.setFillColor(colorDefault);
    }
    return choice;
}

void PromotionBar::setAlignment()
{
    const float adjustment = (1.f - SPRITE_SCALE_FACTOR) * entrySize / 2.f;
    for (int i = 0; i < PROMOTION_CHOICE_RANGE; i++) {
        sf::Vector2f pos = { topLeftCorner.x, topLeftCorner.y + i * entrySize };
        entries[i].circle.setPosition(pos);
        entries[i].sprite.setPosition(pos + sf::Vector2f(adjustment, adjustment));
    }
}