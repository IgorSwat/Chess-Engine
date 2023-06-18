#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "boardconfig.h"
#include "move.h"
#include <unordered_set>
using std::vector;
using std::unordered_set;

class MoveGenerator : public PositionChangedObserver, public ObservableMoves
{
private:
    BoardConfig* config;
    vector< vector<Move2> > legalMoves;
    vector< vector<Move2> > attacksOnly;
    bool checkStates[2] = {false, false};
    unordered_set<int> updatedPieces;
    vector<int> squareDependable[8][8];
    // Helper functions
    void addLegalMove(int pieceID, PieceType type, const sf::Vector2i& targetPos, const MoveMask& mask);
    void addPseudoLegalMove(int pieceID, PieceType type, const sf::Vector2i& targetPos, const MoveMask& mask);
    void removeMoves(int pieceID);
    void registerMove(int pieceID, PieceType type, const sf::Vector2i& targetPos,
                      vector<sf::Vector2i>* checkSavers, bool batteryFlag = false);
    // Specialised move generators
    void generatePawnMoves(Piece* pawn, vector<sf::Vector2i>* checkSavers);
    void generateKnightMoves(Piece* knight, vector<sf::Vector2i>* checkSavers);
    void generateKingMoves(Piece* king, vector<sf::Vector2i>* checkSavers);
    void generateOthersMoves(Piece* other, vector<sf::Vector2i>* checkSavers);
    void dynamicUpdateFromSquare(const sf::Vector2i& pos, std::unordered_set<int>& foundPieces,
                                 vector<sf::Vector2i>** checkCovers);
public:
    MoveGenerator(BoardConfig* cnf) : legalMoves(32, vector<Move2>()), attacksOnly(32, vector<Move2>()) { config = cnf; }
    ~MoveGenerator() {}
    // Static update
    void generatePieceMoves(Piece* piece, vector<sf::Vector2i>* checkSavers);
    void generateAllMoves(COLOR side);
    void generateAllMoves() {generateAllMoves(COLOR::WHITE); generateAllMoves(COLOR::BLACK); resetObservers();}
    virtual void reset() {generateAllMoves();} // Don`t forget about that if needed
    // Dynamic update
    virtual void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos);
    // Moves getters
    const vector< vector<Move2> >& getLegalMoves() const {return legalMoves;}
    const vector< vector<Move2> >& getPseudoLegalMoves() const {return attacksOnly;}
    // Testing
    void showMoves(bool attacks = false) const;
};

#endif // MOVEGENERATOR_H
