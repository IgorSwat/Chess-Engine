#ifndef PIECE_H
#define PIECE_H

#include <vector>
#include <bitset>
#include "misc.h"

typedef std::vector<Square> dirVector;


class PieceBase
{
protected:
    PieceType type;
    bool state;
public:
    PieceBase(PieceType tp) {type = tp; state = true;}
    virtual ~PieceBase() {}
    bool isActive() const {return state;}
    void setState(bool s) {state = s;}
    PieceType getType() const {return type;}
};

class Piece : public PieceBase
{
protected:
    const int pieceID;
    Square square;
    Side color;
    bool bishopAbilities = false;
    bool rookAbilities = false;
    static dirVector pawnDirs;
    static dirVector knightDirs;
    static dirVector bishopDirs;
    static dirVector rookDirs;
    static dirVector queenDirs;
public:
    Piece(PieceType tp, Side clr, int pid, const Square& pos) :PieceBase(tp), color(clr), pieceID(pid), square(pos) {}
    virtual ~Piece() {}
    const Square& getPos() const {return square;}
    int getID() const {return pieceID;}
    Side getColor() const {return color;}
    void setPos(const Square& pos) {square = pos;}
    bool hasBishopAbilities() const {return bishopAbilities;}
    bool hasRookAbilities() const {return rookAbilities;}
    virtual bool isReachable(const Square& targetPos) const = 0;
    virtual bool isAttacking(const Square& position) const {return isReachable(position);}
    virtual Square getMovementVector(const Square& targetPos) const;
    virtual const dirVector& getDirections() const = 0;
};

class Pawn : public Piece
{
public:
    Pawn(Side clr, int pid, const Square& pos) : Piece(PieceType::PAWN, clr, pid, pos) {}
    bool isReachable(const Square& targetPos) const override;
    bool isAttacking(const Square& position) const override;
    Square getMovementVector(const Square& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::pawnDirs;}
};

class Knight : public Piece
{
public:
    Knight(Side clr, int pid, const Square& pos) : Piece(KNIGHT, clr, pid, pos) {}
    bool isReachable(const Square& targetPos) const override;
    Square getMovementVector(const Square& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::knightDirs;}
    static const dirVector& directions() {return Piece::knightDirs;}
};

class Bishop : public Piece
{
public:
    Bishop(Side clr, int pid, const Square& pos) : Piece(BISHOP, clr, pid, pos) {bishopAbilities = true;}
    bool isReachable(const Square& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::bishopDirs;}
    static const dirVector& directions() { return Piece::bishopDirs; }
};

class Rook : public Piece
{
public:
    Rook(Side clr, int pid, const Square& pos) : Piece(ROOK, clr, pid, pos) {rookAbilities = true;}
    bool isReachable(const Square& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::rookDirs;}
    static const dirVector& directions() { return Piece::rookDirs; }
};

class Queen : public Piece
{
public:
    Queen(Side clr, int pid, const Square& pos) : Piece(QUEEN, clr, pid, pos)
        {bishopAbilities = rookAbilities = true;}
    bool isReachable(const Square& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::queenDirs;}
    static const dirVector& directions() {return Piece::queenDirs;}
};

class King : public Piece
{
private:
    static dirVector initialSquares;
public:
    King(Side clr, int pid, const Square& pos) : Piece(PieceType::KING, clr, pid, pos) {}
    bool isReachable(const Square& pos) const override;
    Square getMovementVector(const Square& targetPos) const override;
    const dirVector& getDirections() const {return Piece::queenDirs;}
    static const Square& getInitialSquare(Side col) {return initialSquares[(int)col];}
};

#endif // PIECE_H
