#ifndef CONFIGCHANGE_H
#define CONFIGCHANGE_H

#include "piece.h"
using std::vector;

class BoardConfig;

class ConfigChange
{
public:
    virtual ~ConfigChange() {}
    virtual void applyChange(BoardConfig* config) const = 0;
};

class PlacementChange : public ConfigChange
{
private:
    const int pieceID;
    const Square position;
public:
    PlacementChange(int pieceid, const Square& pos) : pieceID(pieceid), position(pos) {}
    void applyChange(BoardConfig* config) const override;
};

class CastlingChange : public ConfigChange
{
private:
    const bool Wshort;
    const bool Wlong;
    const bool Bshort;
    const bool Blong;
public:
    CastlingChange(bool Ws, bool Wl, bool Bs, bool Bl) : Wshort(Ws), Wlong(Wl), Bshort(Bs), Blong(Bl) {}
    void applyChange(BoardConfig* config) const override;
};

class EnPassantChange : public ConfigChange
{
private:
    const int enPassantRow;
    const int enPassanter1;
    const int enPassanter2;
public:
    EnPassantChange(int epr, int enp1, int enp2) : enPassantRow(epr), enPassanter1(enp1), enPassanter2(enp2) {}
    void applyChange(BoardConfig* config) const override;
};

class SideOnMoveChange : public ConfigChange
{
private:
    Side sideOnMove;
public:
    SideOnMoveChange(Side side) : sideOnMove(side) {}
    void applyChange(BoardConfig* config) const override;
};

class CheckerChange : public ConfigChange
{
private:
    Piece** whiteCheckers;
    Piece** blackCheckers;
    vector<Square>** checkSavers;
public:
    CheckerChange(Piece** wCheckers, Piece** bCheckers, vector<Square>** savers);
    ~CheckerChange() {delete[] whiteCheckers; delete[] blackCheckers; delete[] checkSavers;}
    void applyChange(BoardConfig* config) const override;
};

class PromotionChange : public ConfigChange
{
private:
    int pieceID;
public:
    PromotionChange(int pid) {pieceID = pid;}
    void applyChange(BoardConfig* config) const override;
};

class MoveCountChange : public ConfigChange
{
private:
    int halfMoves;
    int moveNumber;
public:
    MoveCountChange(int hm, int mn) : halfMoves(hm), moveNumber(mn) {}
    void applyChange(BoardConfig* config) const override;
};

#endif // CONFIGCHANGE_H
