#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "chessboardui.h"
#include "navbar.h"
#include "engine.h"

class Chessboard
{
private:
    // Chess logic
    BoardConfig config;
    sf::Vector2i promotionPosition;
    bool clickState = false;
    // Window and graphics members
    sf::RenderWindow* window;
    ChessboardUI* graphics;
    Navbar* navbar;
    Engine* engine;
    std::map<std::string, sf::Texture*> textures;
    // Private functions
    void initTextures();
    bool handleOtherMoves(Piece* movingPiece, const sf::Vector2i& mVector, const sf::Vector2i& targetPos);
    bool handlePawnMoves(Piece* pawn, const sf::Vector2i& movementVector, const sf::Vector2i& targetPos);
    bool handleKingMoves(Piece* king, const sf::Vector2i& movementVector, const sf::Vector2i& targetPos);
    bool handlePromotion();
    void updateNavbar();
public:
    Chessboard(sf::RenderWindow* win, float tileSize = 100.f);
    ~Chessboard();
    bool registerMovement(const sf::Vector2i& initSquare, const sf::Vector2i& targetSquare);
    void testDynamics() const;
    BoardConfig* getCurrentConfig() {return &config;}
    void render(sf::RenderTarget& target);
    void updateBoard();
};

#endif // CHESSBOARD_H
