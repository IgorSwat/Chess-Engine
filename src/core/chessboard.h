#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "../gui/chessboardui.h"
#include "../gui/navbar.h"
#include "../engine/engine.h"

class Chessboard
{
private:
    // Chess logic
    BoardConfig config;
    Square promotionPosition;
    bool clickState = false;
    // Window and graphics members
    sf::RenderWindow* window;
    ChessboardUI* graphics;
    Navbar* navbar;
    Engine* engine;
    // Private functions
    bool handleOtherMoves(Piece* movingPiece, const Square& mVector, const Square& targetPos);
    bool handlePawnMoves(Piece* pawn, const Square& movementVector, const Square& targetPos);
    bool handleKingMoves(Piece* king, const Square& movementVector, const Square& targetPos);
    bool handlePromotion();
    void updateNavbar();
public:
    Chessboard(sf::RenderWindow* win, float tileSize = 100.f);
    ~Chessboard();
    bool registerMovement(const Square& initSquare, const Square& targetSquare);
    void testDynamics() const;
    BoardConfig* getCurrentConfig() {return &config;}
    void render(sf::RenderTarget& target);
    void updateBoard();
};

#endif // CHESSBOARD_H
