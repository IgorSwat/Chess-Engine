#include "navbar.h"
#include "textures.h"

using BoardTextures::textures;

Navbar::Navbar(const sf::Vector2f& initialPos, const sf::Vector2f& initialSize)
    : leftCorner(initialPos), size(initialSize)
{
    for (int i = 0; i < 3; i++)
    {
        sf::Vector2f entryPos(leftCorner.x + size.x / 2.f - entrySize / 2.f, (i + 1) * objectSpacing + i * entrySize);
        entries[i] = new MoveEntry(entryPos, entrySize, textures["Wpawn"]);
        moves[i] = new Move2();
    }
    resetButton = new Button(navButtonsSize, textures["refresh"]);
    resetButton->setPosition(leftCorner.x + size.x / 2.f - (navButtonsCount + 1) * navButtonsSize / 2.f,
                            leftCorner.y + size.y - objectSpacing * 1.5f);
    backButton = new Button(navButtonsSize, textures["back"]);
    backButton->setPosition(leftCorner.x + size.x / 2.f + (navButtonsCount) * navButtonsSize / 3.f,
                            leftCorner.y + size.y - objectSpacing * 1.5f);
    separator = new sf::RectangleShape(sf::Vector2f(size.x, 4.f));
    separator->setPosition(leftCorner.x, leftCorner.y + size.y - objectSpacing * 2.2f);
    separator->setFillColor(sf::Color(0, 0, 0, 160));
}

Navbar::~Navbar()
{
    for (int i = 0; i < 3; i++)
    {
        delete entries[i];
        delete moves[i];
    }
    delete backButton;
    delete resetButton;
    delete separator;
}

Navbar::NavbarState Navbar::update(sf::RenderWindow* window)
{
    NavbarState navState = {ButtonType::NONE, -1};
    for (int i = 0 ; i < 3; i++)
    {
        ButtonState bstate = entries[i]->update(window);
        switch (bstate)
        {
        case ButtonState::MARKED:
            navState.type = ButtonType::ENTRYMARKED;
            navState.entryID = i;
            break;
        case ButtonState::CLICKED:
            navState.type = ButtonType::ENTRYCLICKED;
            navState.entryID = i;
            break;
        default:
            break;
        }
    }
    if (resetButton->update(window) == ButtonState::CLICKED)
        navState.type = ButtonType::RESET;
    if (backButton->update(window) == ButtonState::CLICKED)
        navState.type = ButtonType::BACK;
    return navState;
}

void Navbar::setMove(const Move2& move, int moveID)
{
    delete moves[moveID];
    moves[moveID] = new Move2(move);
    Side side = move.move.pieceID < 16 ? WHITE : BLACK;
    std::string textureName = "";
    textureName += side == WHITE ? "W" : "B";
    switch (moves[moveID]->move.pieceType)
    {
    case PAWN:
        textureName += "pawn";
        break;
    case KNIGHT:
        textureName += "knight";
        break;
    case BISHOP:
        textureName += "bishop";
        break;
    case ROOK:
        textureName += "rook";
        break;
    case QUEEN:
        textureName += "queen";
        break;
    case KING:
        textureName += "king";
        break;
    }
    entries[moveID]->setData(textures[textureName], moves[moveID]->getMoveNotation(), Moves::writeEvaluation(moves[moveID]->eval));
}

void Navbar::render(sf::RenderTarget& target)
{
    for (int i = 0; i < 3; i++)
        entries[i]->render(target);
    resetButton->render(target);
    backButton->render(target);
    target.draw(*separator);
}

