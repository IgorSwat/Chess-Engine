#ifndef NAVBAR_H
#define NAVBAR_H

#include "moveentry.h"
#include "move.h"

enum class ButtonType {NONE, BACK, RESET, ENTRYMARKED, ENTRYCLICKED};

class Navbar
{
private:
    sf::Vector2f leftCorner;
    sf::Vector2f size;
    MoveEntry* entries[3];
    Move* moves[3];
    Button* resetButton;
    Button* backButton;
    sf::RectangleShape* separator;
    std::map<std::string, sf::Texture*>& textures;
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
    Navbar(const sf::Vector2f& initialPos, const sf::Vector2f& initialSize, std::map<std::string, sf::Texture*>& txts);
    ~Navbar();
    NavbarState update(sf::RenderWindow* window);
    void setMove(const Move& move, int moveID);
    Move getMove(int moveID) const {return *moves[moveID];}
    void render(sf::RenderTarget& target);
};

#endif // NAVBAR_H
