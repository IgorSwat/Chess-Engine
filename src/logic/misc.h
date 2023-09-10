#pragma once

/// Board square representation
class Square
{
public:
    int x;
    int y;
    Square(int x_ = 8, int y_ = 8) : x(x_), y(y_) {}
    Square operator+(const Square& other) const { return { x + other.x, y + other.y }; }
    Square& operator+=(const Square& other) { x += other.x; y += other.y; return *this; }
    Square operator-(const Square& other) const { return { x - other.x, y - other.y }; }
    Square& operator-=(const Square& other) { x -= other.x; y -= other.y; return *this; }
    Square operator*(int r) const { return { x * r, y * r }; }
    Square& operator*=(int r) { x *= r; y *= r; return *this; }
    friend Square operator*(int r, const Square& square) { return square * r; }
    Square operator-() const { return { -x, -y }; }
    bool operator==(const Square& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Square& other) const { return x != other.x || y != other.y; }
};

enum SquareColors { LIGHT_SQUARE = 0, DARK_SQUARE };

static SquareColors colorsMap[8][8]{
    {LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE},
    {DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE},
    {LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE},
    {DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE},
    {LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE},
    {DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE},
    {LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE},
    {DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE, DARK_SQUARE, LIGHT_SQUARE}
};


/// Piece types & board sides
enum Side { WHITE = 0, BLACK = 1 };
enum PieceType { PAWN = 0, KNIGHT, BISHOP, ROOK, QUEEN, KING, INACTIVE };

static Side opposition[2] { BLACK, WHITE };


// Castling
enum CastleType {SHORT_CASTLE = 0, LONG_CASTLE};



/// Piece types ordering
inline int mapPieceType(PieceType type) {return (int)type - 1;}
int mapPieceTypeByValue(PieceType type);



/// Factors & evaluation
class Factor
{
public:
    Factor(short beg, short end) : startValue(beg), endValue(end) {}
    short startValue;
    short endValue;
};

int interpolation(const int& begVal, const int& endVal, const float& w, float (*func)(float));
int interpolation(const Factor& vals, const float& w, float (*func)(float));
float mobilityProgressFunction(float x);