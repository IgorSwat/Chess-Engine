#include "promotionbar.h"
#include "textures.h"

using BoardTextures::textures;

PromotionChoice PromotionBar::choices[4] = {PromotionChoice::QUEEN, PromotionChoice::ROOK,
                                PromotionChoice::BISHOP, PromotionChoice::KNIGHT};

PromotionBar::PromotionBar(Side side)
{
    normal = sf::Color(115, 119, 128);
    onCursor = sf::Color(149, 151, 158);
    onClick = sf::Color(190, 202, 232);
    const float radius = tileSize / 2.f;
    for (int i = 0; i < 4; i++)
    {
        circles[i] = new sf::CircleShape(radius);
        circles[i]->setFillColor(normal);
        sprites[i] = new sf::Sprite();
        sprites[i]->setScale(scaleFactor, scaleFactor);
    }
    if (side == WHITE)
    {
        sprites[0]->setTexture(textures["Wqueen"]);
        sprites[1]->setTexture(textures["Wrook"]);
        sprites[2]->setTexture(textures["Wbishop"]);
        sprites[3]->setTexture(textures["Wknight"]);
    }
    else
    {
        sprites[0]->setTexture(textures["Bqueen"]);
        sprites[1]->setTexture(textures["Brook"]);
        sprites[2]->setTexture(textures["Bbishop"]);
        sprites[3]->setTexture(textures["Bknight"]);
    }
}

PromotionBar::~PromotionBar()
{
    for (int i = 0; i < 4; i++)
    {
        delete circles[i];
        delete sprites[i];
    }
}

void PromotionBar::setAlignment()
{
    const float adjustment = (1.f - scaleFactor) * tileSize / 2.f;
    for (int i = 0; i < 4; i++)
    {
        sf::Vector2f pos(position.x, position.y + i * tileSize);
        circles[i]->setPosition(pos);
        sprites[i]->setPosition(pos.x + adjustment, pos.y + adjustment);
    }
}

void PromotionBar::setPosition(const sf::Vector2f& pos)
{
    position = pos;
    setAlignment();
}

void PromotionBar::setPosition(float xPos, float yPos)
{
    setPosition(sf::Vector2f(xPos, yPos));
}

PromotionChoice PromotionBar::update(sf::RenderWindow* window)
{
    PromotionChoice choice = PromotionChoice::NONE;
    for (int i = 0; i < 4; i++)
    {
        sf::CircleShape* circle = circles[i];
        sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
        sf::Vector2f mapped(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        if (circle->getGlobalBounds().contains(mapped))
        {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                circle->setFillColor(onClick);
                choice = choices[i];
            }
            else
                circle->setFillColor(onCursor);
        }
        else
            circle->setFillColor(normal);
    }
    return choice;
}

void PromotionBar::render(sf::RenderTarget& target)
{
    for (int i = 0; i < 4; i++)
    {
        target.draw(*circles[i]);
        target.draw(*sprites[i]);
    }
}

