#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "progressstack.h"
#include "fenreader.h"
#include "observable.h"
#include <vector>

using squaresVecRef = const std::vector<Square>&;

class MoveGenerator;

class BoardConfig : public ObservablePosition
{
private:
    // Pieces and logic
    King* whiteKing;
    King* blackKing;
    Piece* pieces[32];
    Piece* table[8][8];
    std::vector<Square> piecePositions[2][5] {};    // side - piece type
    Side sideOnMove;
    unsigned int halfMoves = 0;
    unsigned int moveNumber = 1;
    int enPassantLine = -2;
    int enPassanter1 = -1;
    int enPassanter2 = -1;
    bool Wshort;
    bool Wlong;
    bool Bshort;
    bool Blong;
    MoveGenerator* generator = nullptr;
    // Attack tables
    short attacksTable[8][8][2];
    // Pin tables
    Square pinVectors[32];
    vector<int> pinnedPieces[2];
    // Checks tables
    Piece* whiteCkecker[2];
    Piece* blackCkecker[2];
    vector<Square>* safeMoves[2];
    // Backwards update
    ProgressStack boardProgress;
    // Precomputed static elements
    static Square directionalVectors[15][15];
    static vector<Square> checkSavers[8][8][8][8];
    static vector<Square> emptySaver;
    static bool directionalComputed;
    static bool saversComputed;
    static constexpr int blackID = 16;
    static constexpr int kingID[2] { 0, 16 };
    // FEN parsing
    FenReader FENreader;
    // Connected engines
    vector<EngineObserver*> connectedEngines;
    // Helper functions
    static void initDirectionalVectors();
    static void initCheckSavers();
    static Square toDirectionalVector(const Square& initPos, const Square& targetPos);
    static Square toDirectionalVector(const Square& mv);
    void placePiece(const Square& pos, int pieceID, bool observableFlag = true); // Safe version
    void placePiece(const Square& pos, int pieceID, PieceType type, bool observableFlag = true); // Needs control of args
    void clearBoard();
    Piece* nextInDirection(const Square& start, const Square& mv, Piece* except);
    Piece* nextInDirection(const Square& start, const Square& mv, const Square& end, Piece* except);
    bool follows(const Square& pos, const Square& mv, const Square& target);
    // Pinning handlers
    void searchForPins(Piece* king, const Square& dir);
    void updatePinsFromKing(Piece* king, const Square& square);
    void updatePinsFromSquare(const Square& square);
    void resetPins();
    // Dynamic board parameters update
    void updateSideOnMove();
    void updateMoveCounts(Piece* movedPiece, bool captureFlag);
    void updateEnPassant(Piece* piece, bool captureFlag, const Square& oldPos, const Square& newPos);
    void updateCastling(Piece* movedPiece, const Square& oldPos);
    void updateChecks(Piece* movedPiece, const Square& oldPos);
    void updatePinsDynamic(int pieceID, const Square& oldPos, const Square& newPos);
    void updatePiecePlacement(Side side, PieceType type, const Square& oldPos, const Square& newPos);
    void updatePinsStatic();
    void updateChecksForSide(Side side);
    void updateChecksStatic();
public:
    // Constructors and destructors
    BoardConfig();
    BoardConfig(const BoardConfig& config);
    BoardConfig& operator=(const BoardConfig& config);
    ~BoardConfig();
    // Methods for reloading global state
    void setToDefault();
    bool setFromFEN(const std::string& FEN);
    void connectEngine(EngineObserver* engine);
    void disconnectEngine(EngineObserver* engine);
    void updateEngines();
    // Moving and removing pieces
    void movePiece(int pieceID, const Square& targetPos, PieceType promoteTo = PieceType::PAWN);
    void removePiece(int pieceID);
    int enPassant(Piece* pawn); // Returns ID of captured pawn
    void castle(Piece* king, const Square& mv);
    Piece* promote(int pawnID, PieceType newType);  // returns the promoted piece
    bool undoLastMove();
    // Getters
    Side getSideOnMove() const {return sideOnMove;}
    Piece* getKing(Side side) { return pieces[kingID[side]]; }
    Piece* getPiece(int row, int col) { return table[row][col]; }
    Piece* getPiece(const Square& pos) {return table[pos.y][pos.x];}
    Piece* getPiece(int pieceID) {return pieces[pieceID];}
    Piece* getKingUnderCheck() const;
    int getMoveCount() const {return moveNumber;}
    squaresVecRef getPiecePlacement(Side side, PieceType type) const { return piecePositions[side][type]; }
    // External data handlers
    void addAttack(const Square& position, int side) {attacksTable[position.y][position.x][side] += 1;}
    void removeAttack(const Square& position, int side) {attacksTable[position.y][position.x][side] -= 1;}
    void clearAttacksTable();
    // Colision check
    Piece* checkCollisions(const Square& initPos, const Square& movement, const Square& targetPos) { return nextInDirection(initPos, movement, targetPos, nullptr); }
    // Checks control
    Square isPinned(Piece* piece);
    const Square& isPinned2(Piece* piece) {return pinVectors[piece->getID()];}
    Piece* isChecked(const Square& square, Side color); // Checks if the square is checked by any piece from side of color
    // Simpler, more efficient version - available only with MoveGenerator & SquareControl connected
    bool isChecked2(const Square& square, Side color) const {return attacksTable[square.y][square.x][(int)color] > 0;}
    bool isCoveringChecks(Piece* movingPiece, const Square& targetPos);
    vector<Square>* getSquaresCoveringChecks(Side side) {return safeMoves[side];}
    bool isKingChecked(Side side);
    // En passant
    int getEnPassantLine() const {return enPassantLine;}
    bool isEnPassantPossible(Piece* pawn);
    Square getEnPassantVector(Piece* pawn);
    // Castling
    int isCastlingAvailable(Piece* king, const Square& mv);   // returns the id of castling rook if castling is available
    int isCastlingAvailable2(Piece* king, const Square& mv);
    void connectMoveGenerator(MoveGenerator* gen) { generator = gen; }
    void showAllPieces() const;
    // Static functions
    static bool isPawn(Piece* piece) {return piece->getType() == PieceType::PAWN;}
    static bool isKnight(Piece* piece) {return piece->getType() == KNIGHT;}
    static bool isBishop(Piece* piece) {return piece->getType() == BISHOP;}
    static bool isRook(Piece* piece) {return piece->getType() == ROOK;}
    static bool isQueen(Piece* piece) {return piece->getType() == QUEEN;}
    static bool isKing(Piece* piece) {return piece->getType() == PieceType::KING;}
    static const Square& toDirectionalVector2(const Square& initPos, const Square& targetPos);
    static const Square& toDirectionalVector2(const Square& mv) { return directionalVectors[7 - mv.x][7 - mv.y]; }
    // Friends
    friend bool FenReader::parseToConfig(BoardConfig* config) const;
    friend void PlacementChange::applyChange(BoardConfig* config) const;
    friend void CastlingChange::applyChange(BoardConfig* config) const;
    friend void EnPassantChange::applyChange(BoardConfig* config) const;
    friend void SideOnMoveChange::applyChange(BoardConfig* config) const;
    friend void CheckerChange::applyChange(BoardConfig* config) const;
    friend void PromotionChange::applyChange(BoardConfig* config) const;
    friend void MoveCountChange::applyChange(BoardConfig* config) const;
};

inline bool isCorrectSquare(const Square& square)
{
    return square.x >= 0 && square.x < 8 && square.y >= 0 && square.y < 8;
}

#endif // BOARDCONFIG_H
