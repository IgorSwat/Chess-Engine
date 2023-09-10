#pragma once

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
    void handleNormalMoves(const Piece* movingPiece, const Move& move);
    void handlePawnMoves(const Piece* movingPiece, const Move& move);
    void handleKingMoves(const Piece* movingPiece, const Move& move);
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