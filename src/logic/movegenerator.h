#pragma once

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
    void setListKeys(int pieceID, int directionCount, const DirectionsVec& directions);
    void addLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, int flags);
    void addPseudoLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, int flags);
    void removeMoves(int pieceID, const Square& dir);
    void registerMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos,
        const vector<Square>* checkSavers, bool batteryFlag = false);
    // Specialised move generators
    void generatePawnMoves(const Piece* pawn, const vector<Square>* checkSavers);
    void generatePawnMoves(const Square& dir, const Piece* pawn, const vector<Square>* checkSavers);
    void generateKnightMoves(const Piece* knight, const vector<Square>* checkSavers);
    void generateKnightMoves(const Square& dir, const Piece* knight, const vector<Square>* checkSavers);
    void generateKingMoves(const Piece* king, const vector<Square>* checkSavers);
    void generateOthersMoves(const Piece* other, const vector<Square>* checkSavers);
    void generateOthersMoves(const Square& dir, const Piece* other, const vector<Square>* checkSavers);
    void dynamicUpdateFromSquare(const Square& pos, std::unordered_set<int>& foundPieces,
                                 const SquaresVec* const* checkCovers);
public:
    MoveGenerator(BoardConfig* cnf);
    ~MoveGenerator() {}
    // Static update
    void configureMoveList(int pieceID);
    void generatePieceMoves(const Piece* piece, const vector<Square>* checkSavers);
    void generatePieceMoves(const Square& dir, const Piece* piece, const vector<Square>* checkSavers);
    void removeMoves(int pieceID);
    void generateNonKingMoves(Side side);
    void generateAllMoves();
    // Dynamic update
    virtual void updateByMove(int pieceID, const Square& oldPos, const Square& newPos) override;
    void updateObserversByremoval(int pieceID) { updateObserversByRemoval(pieceID, legalMoves[pieceID]); updateObserversByRemoval(pieceID, attacksOnly[pieceID]); }
    // Moves getters
    const MoveList* getLegalMoves() {return legalMoves;}
    const MoveList* getPseudoLegalMoves() {return attacksOnly;}
    // Testing
    void showMoves(bool attacks = false);
};