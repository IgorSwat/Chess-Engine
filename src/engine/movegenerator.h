#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "../logic/boardconfig.h"
#include "../logic/move.h"
#include "../logic/movelist.h"
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
    void addLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, const MoveMask& mask);
    void addPseudoLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, const MoveMask& mask);
    void removeMoves(int pieceID, const Square& dir);
    void registerMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos,
        vector<Square>* checkSavers, bool batteryFlag = false);
    // Specialised move generators
    void generatePawnMoves(Piece* pawn, vector<Square>* checkSavers);
    void generatePawnMoves(const Square& dir, Piece* pawn, vector<Square>* checkSavers);
    void generateKnightMoves(Piece* knight, vector<Square>* checkSavers);
    void generateKnightMoves(const Square& dir, Piece* knight, vector<Square>* checkSavers);
    void generateKingMoves(Piece* king, vector<Square>* checkSavers);
    void generateOthersMoves(Piece* other, vector<Square>* checkSavers);
    void generateOthersMoves(const Square& dir, Piece* other, vector<Square>* checkSavers);
    void dynamicUpdateFromSquare(const Square& pos, std::unordered_set<int>& foundPieces,
                                 vector<Square>** checkCovers);
public:
    MoveGenerator(BoardConfig* cnf) { config = cnf; initializeMoveLists(); }
    ~MoveGenerator() {}
    // Static update
    void configureMoveList(int pieceID);
    void generatePieceMoves(Piece* piece, vector<Square>* checkSavers);
    void generatePieceMoves(const Square& dir, Piece* piece, vector<Square>* checkSavers);
    void removeMoves(int pieceID);
    void generateAllMoves(Side side);
    void generateAllMoves() { generateAllMoves(WHITE); generateAllMoves(BLACK);}
    // Dynamic update
    virtual void updateByMove(int pieceID, const Square& oldPos, const Square& newPos);
    void updateObserversByremoval(int pieceID) { updateObserversByRemoval(pieceID, legalMoves[pieceID]); updateObserversByRemoval(pieceID, attacksOnly[pieceID]); }
    // Moves getters
    const MoveList* getLegalMoves() {return legalMoves;}
    const MoveList* getPseudoLegalMoves() {return attacksOnly;}
    // Testing
    void showMoves(bool attacks = false);
};

#endif // MOVEGENERATOR_H
