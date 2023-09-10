#pragma once

#include "moveentry.h"
#include "../logic/move.h"

enum class ButtonType {NONE, BACK, RESET, ENTRYMARKED, ENTRYCLICKED};

class Navbar
{
private:
    sf::Vector2f leftCorner;
    sf::Vector2f size;
    MoveEntry* entries[3];
    Move2* moves[3];
    Button* resetButton;
    Button* backButton;
    sf::RectangleShape* separator;
    static constexpr float entrySize = 120.f;
    static constexpr float navButtonsSize = 40.f;
    static constexpr float objectSpacing = 50.f;
    static constexpr int navButtonsCount = 2;
public:
    struct NavbarState
    {
        ButtonType type;
        int entryID;
    };
    Navbar(const sf::Vector2f& initialPos, const sf::Vector2f& initialSize);
    ~Navbar();
    NavbarState update(sf::RenderWindow* window);
    void setMove(const Move2& move, int moveID);
    const Move2& getMove(int moveID) const {return *moves[moveID];}
    void render(sf::RenderTarget& target);
};