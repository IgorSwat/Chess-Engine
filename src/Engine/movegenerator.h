#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "boardconfig.h"
#include "move.h"
#include "movelist.h"
#include <unordered_set>
using std::vector;
using std::unordered_set;

class MoveGenerator : public PositionChangedObserver, public ObservableMoves
{
private:
    BoardConfig* config;
    MoveList legalMoves[32] {};
    MoveList attacksOnly[32] {};
    bool checkStates[2] {false, false};
    unordered_set<int> updatedPieces;
    vector<int> squareDependable[8][8];
    // Helper functions
    void initializeMoveLists();
    void setListKeys(int pieceID, int directionCount, const dirVector& directions);
    void addLegalMove(const sf::Vector2i& dir, int pieceID, PieceType type, const sf::Vector2i& targetPos, const MoveMask& mask);
    void addPseudoLegalMove(const sf::Vector2i& dir, int pieceID, PieceType type, const sf::Vector2i& targetPos, const MoveMask& mask);
    void removeMoves(int pieceID, const sf::Vector2i& dir);
    void registerMove(const sf::Vector2i& dir, int pieceID, PieceType type, const sf::Vector2i& targetPos,
        vector<sf::Vector2i>* checkSavers, bool batteryFlag = false);
    // Specialised move generators
    void generatePawnMoves(Piece* pawn, vector<sf::Vector2i>* checkSavers);
    void generatePawnMoves(const sf::Vector2i& dir, Piece* pawn, vector<sf::Vector2i>* checkSavers);
    void generateKnightMoves(Piece* knight, vector<sf::Vector2i>* checkSavers);
    void generateKnightMoves(const sf::Vector2i& dir, Piece* knight, vector<sf::Vector2i>* checkSavers);
    void generateKingMoves(Piece* king, vector<sf::Vector2i>* checkSavers);
    void generateOthersMoves(Piece* other, vector<sf::Vector2i>* checkSavers);
    void generateOthersMoves(const sf::Vector2i& dir, Piece* other, vector<sf::Vector2i>* checkSavers);
    void dynamicUpdateFromSquare(const sf::Vector2i& pos, std::unordered_set<int>& foundPieces,
                                 vector<sf::Vector2i>** checkCovers);
public:
    MoveGenerator(BoardConfig* cnf) { config = cnf; initializeMoveLists(); }
    ~MoveGenerator() {}
    // Static update
    void configureMoveList(int pieceID);
    void generatePieceMoves(Piece* piece, vector<sf::Vector2i>* checkSavers);
    void generatePieceMoves(const sf::Vector2i& dir, Piece* piece, vector<sf::Vector2i>* checkSavers);
    void removeMoves(int pieceID);
    void generateAllMoves(COLOR side);
    void generateAllMoves() {generateAllMoves(COLOR::WHITE); generateAllMoves(COLOR::BLACK); resetObservers();}
    virtual void reset() {generateAllMoves();} // Don`t forget about that if needed
    // Dynamic update
    virtual void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos);
    void updateObserversByremoval(int pieceID) { updateObserversByRemoval(legalMoves[pieceID]); updateObserversByRemoval(attacksOnly[pieceID]); }
    // Moves getters
    const MoveList* getLegalMoves() {return legalMoves;}
    const MoveList* getPseudoLegalMoves() {return attacksOnly;}
    // Testing
    void showMoves(bool attacks = false);
};

#endif // MOVEGENERATOR_H
