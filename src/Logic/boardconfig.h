#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "progressstack.h"
#include "fenreader.h"
#include "observable.h"

using sf::Vector2i;
class MoveGenerator;

class BoardConfig : public ObservablePosition
{
private:
    // Pieces and logic
    King* whiteKing;
    King* blackKing;
    Piece* pieces[32];
    Piece* table[8][8];
    COLOR sideOnMove;
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
    Vector2i pinVectors[32];
    vector<int> pinnedPieces[2];
    // Checks tables
    Piece* whiteCkecker[2];
    Piece* blackCkecker[2];
    vector<Vector2i>* safeMoves[2];
    // Backwards update
    ProgressStack boardProgress;
    // Precomputed static elements
    static Vector2i directionalVectors[15][15];
    static vector<Vector2i> checkSavers[8][8][8][8];
    static vector<Vector2i> emptySaver;
    static bool directionalComputed;
    static bool saversComputed;
    static constexpr int blackID = 16;
    // FEN parsing
    FenReader FENreader;
    // Helper functions
    static void initDirectionalVectors();
    static void initCheckSavers();
    static Vector2i toDirectionalVector(const sf::Vector2i& initPos, const sf::Vector2i& targetPos);
    static Vector2i toDirectionalVector(const sf::Vector2i& mv);
    void placePiece(const sf::Vector2i& pos, int pieceID, bool observableFlag = true); // Safe version
    void placePiece(const sf::Vector2i& pos, int pieceID, PieceType type, bool observableFlag = true); // Needs control of args
    void clearBoard();
    Piece* nextInDirection(const sf::Vector2i& start, const sf::Vector2i& mv, Piece* except);
    Piece* nextInDirection(const sf::Vector2i& start, const sf::Vector2i& mv, const sf::Vector2i& end, Piece* except);
    bool follows(const sf::Vector2i& pos, const sf::Vector2i& mv, const sf::Vector2i& target);
    // Pinning handlers
    void searchForPins(Piece* king, const Vector2i& dir);
    void updatePinsFromKing(Piece* king, const Vector2i& square);
    void updatePinsFromSquare(const Vector2i& square);
    void resetPins();
    // Dynamic board parameters update
    void updateSideOnMove();
    void updateMoveCounts(Piece* movedPiece, bool captureFlag);
    void updateEnPassant(Piece* piece, bool captureFlag, const sf::Vector2i& oldPos, const sf::Vector2i& newPos);
    void updateCastling(Piece* movedPiece, const sf::Vector2i& oldPos);
    void updateChecks(Piece* movedPiece, const sf::Vector2i& oldPos);
    void updatePinsDynamic(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos);
    void updatePinsStatic();
    void updateChecksForSide(COLOR side);
    void updateChecksStatic();
public:
    // Constructors and destructors
    BoardConfig();
    ~BoardConfig();
    // Methods for reloading global state
    void setToDefault();
    bool setFromFEN(const std::string& FEN);
    // Moving and removing pieces
    void movePiece(int pieceID, const sf::Vector2i& targetPos, PieceType promoteTo = PieceType::PAWN);
    void removePiece(int pieceID);
    int enPassant(Piece* pawn); // Returns ID of captured pawn
    void castle(Piece* king, const sf::Vector2i& mv);
    Piece* promote(int pawnID, PieceType newType);  // returns the promoted piece
    bool undoLastMove();
    // Getters
    COLOR getSideOnMove() const {return sideOnMove;}
    Piece* getPiece(const sf::Vector2i& pos) {return table[pos.y][pos.x];}
    Piece* getPiece(int pieceID) {return pieces[pieceID];}
    Piece* getKingUnderCheck() const;
    int getMoveCount() const {return moveNumber;}
    // External data handlers
    void addAttack(const sf::Vector2i& position, int side) {attacksTable[position.y][position.x][side] += 1;}
    void removeAttack(const sf::Vector2i& position, int side) {attacksTable[position.y][position.x][side] -= 1;}
    void clearAttacksTable();
    // Colision check
    Piece* checkCollisions(const sf::Vector2i& initPos, const sf::Vector2i& movement, const sf::Vector2i& targetPos);
    // Checks control
    Vector2i isPinned(Piece* piece);
    const Vector2i& isPinned2(Piece* piece) {return pinVectors[piece->getID()];}
    Piece* isChecked(const sf::Vector2i& square, COLOR color); // Checks if the square is checked by any piece from side of color
    // Simpler, more efficient version - available only with MoveGenerator & SquareControl connected
    bool isChecked2(const sf::Vector2i& square, COLOR color) const {return attacksTable[square.y][square.x][(int)color] > 0;}
    bool isCoveringChecks(Piece* movingPiece, const sf::Vector2i& targetPos);
    vector<Vector2i>* getSquaresCoveringChecks(COLOR side) {return safeMoves[(int)side];}
    bool isKingChecked(COLOR side);
    // En passant
    int getEnPassantLine() const {return enPassantLine;}
    bool isEnPassantPossible(Piece* pawn);
    sf::Vector2i getEnPassantVector(Piece* pawn);
    // Castling
    int isCastlingAvailable(Piece* king, const sf::Vector2i& mv);   // returns the id of castling rook if castling is available
    int isCastlingAvailable2(Piece* king, const sf::Vector2i& mv);
    void connectMoveGenerator(MoveGenerator* gen) { generator = gen; }
    void showAllPieces() const;
    // Static functions
    static bool isPawn(Piece* piece) {return piece->getType() == PieceType::PAWN;}
    static bool isKnight(Piece* piece) {return piece->getType() == PieceType::KNIGHT;}
    static bool isBishop(Piece* piece) {return piece->getType() == PieceType::BISHOP;}
    static bool isRook(Piece* piece) {return piece->getType() == PieceType::ROOK;}
    static bool isQueen(Piece* piece) {return piece->getType() == PieceType::QUEEN;}
    static bool isKing(Piece* piece) {return piece->getType() == PieceType::KING;}
    static const Vector2i& toDirectionalVector2(const sf::Vector2i& initPos, const sf::Vector2i& targetPos);
    static const Vector2i& toDirectionalVector2(const sf::Vector2i& mv) { return directionalVectors[7 - mv.x][7 - mv.y]; }
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

inline bool isCorrectSquare(const sf::Vector2i& square)
{
    return square.x >= 0 && square.x < 8 && square.y >= 0 && square.y < 8;
}

#endif // BOARDCONFIG_H
