#ifndef PIECEIMAGE_H_INCLUDED
#define PIECEIMAGE_H_INCLUDED

#include "piece.h"

class PieceImage : public PieceBase
{
private:
    sf::Vector2f position;
    sf::Sprite image;
public:
    PieceImage(sf::Texture* texture, PieceType pieceType, const sf::Vector2f& initialPos) : PieceBase(pieceType)
    {
        position = initialPos;
        image.setPosition(position);
        image.setTexture(*texture);
    }
    void setType(PieceType pieceType) {type = pieceType;}
    void updateOrigin(const sf::Vector2i& mousePos)
    {
        image.setOrigin(sf::Vector2f((float)mousePos.x - position.x, (float)mousePos.y - position.y));
    }
    void changePositionTemp(const sf::Vector2f& pos) {image.setPosition(pos);}
    void changePositionPerm(const sf::Vector2f& pos) {position = pos; resetPosition();}
    void resetPosition() {image.setOrigin(sf::Vector2f(0.f, 0.f)); image.setPosition(position);}
    void setTexture(sf::Texture* texture) {image.setTexture(*texture);}
    sf::Vector2f getLastPosition() const {return position;}
    const sf::Sprite& getImage() const {return image;}
};

#endif // PIECEIMAGE_H_INCLUDED
