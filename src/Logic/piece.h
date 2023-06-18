#ifndef PIECE_H
#define PIECE_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <bitset>

typedef std::vector<sf::Vector2i> dirVector;

enum class COLOR {WHITE = 0, BLACK = 1};
enum PieceType {PAWN = 0, KNIGHT, BISHOP, ROOK, QUEEN, KING, INACTIVE};

COLOR operator++(COLOR color);

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
    sf::Vector2i square;
    COLOR color;
    bool bishopAbilities = false;
    bool rookAbilities = false;
    static dirVector pawnDirs;
    static dirVector knightDirs;
    static dirVector bishopDirs;
    static dirVector rookDirs;
    static dirVector queenDirs;
public:
    Piece(PieceType tp, COLOR clr, int pid, const sf::Vector2i& pos) : PieceBase(tp), pieceID(pid), square(pos)
        {color = clr; state = true;}
    virtual ~Piece() {}
    const sf::Vector2i& getPos() const {return square;}
    int getID() const {return pieceID;}
    COLOR getColor() const {return color;}
    void setPos(const sf::Vector2i& pos) {square = pos;}
    bool hasBishopAbilities() const {return bishopAbilities;}
    bool hasRookAbilities() const {return rookAbilities;}
    virtual bool isReachable(const sf::Vector2i& targetPos) const = 0;
    virtual bool isAttacking(const sf::Vector2i& position) const {return isReachable(position);}
    virtual sf::Vector2i getMovementVector(const sf::Vector2i& targetPos) const;
    virtual const dirVector& getDirections() const = 0;
};

class Pawn : public Piece
{
public:
    Pawn(COLOR clr, int pid, const sf::Vector2i& pos) : Piece(PieceType::PAWN, clr, pid, pos) {}
    bool isReachable(const sf::Vector2i& targetPos) const override;
    bool isAttacking(const sf::Vector2i& position) const override;
    sf::Vector2i getMovementVector(const sf::Vector2i& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::pawnDirs;}
};

class Knight : public Piece
{
public:
    Knight(COLOR clr, int pid, const sf::Vector2i& pos) : Piece(PieceType::KNIGHT, clr, pid, pos) {}
    bool isReachable(const sf::Vector2i& targetPos) const override;
    sf::Vector2i getMovementVector(const sf::Vector2i& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::knightDirs;}
    static const dirVector& directions() {return Piece::knightDirs;}
};

class Bishop : public Piece
{
public:
    Bishop(COLOR clr, int pid, const sf::Vector2i& pos) : Piece(PieceType::BISHOP, clr, pid, pos) {bishopAbilities = true;}
    bool isReachable(const sf::Vector2i& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::bishopDirs;}
};

class Rook : public Piece
{
public:
    Rook(COLOR clr, int pid, const sf::Vector2i& pos) : Piece(PieceType::ROOK, clr, pid, pos) {rookAbilities = true;}
    bool isReachable(const sf::Vector2i& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::rookDirs;}
};

class Queen : public Piece
{
public:
    Queen(COLOR clr, int pid, const sf::Vector2i& pos) : Piece(PieceType::QUEEN, clr, pid, pos)
        {bishopAbilities = rookAbilities = true;}
    bool isReachable(const sf::Vector2i& targetPos) const override;
    const dirVector& getDirections() const override {return Piece::queenDirs;}
    static const dirVector& directions() {return Piece::queenDirs;}
};

class King : public Piece
{
private:
    static dirVector initialSquares;
public:
    King(COLOR clr, int pid, const sf::Vector2i& pos) : Piece(PieceType::KING, clr, pid, pos) {}
    bool isReachable(const sf::Vector2i& pos) const override;
    sf::Vector2i getMovementVector(const sf::Vector2i& targetPos) const override;
    const dirVector& getDirections() const {return Piece::queenDirs;}
    static const sf::Vector2i& getInitialSquare(COLOR col) {return initialSquares[(int)col];}
};

#endif // PIECE_H
