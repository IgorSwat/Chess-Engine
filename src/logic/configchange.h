#pragma once

#include "piece.h"
#include <bitset>
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
    const std::bitset<4> castlingRights;
public:
    CastlingChange(const std::bitset<4>& castling) : castlingRights(castling) {}
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

class CheckerChange : public ConfigChange
{
private:
    Piece* checkers[2][2];
    vector<Square>* checkSavers[2];
public:
    CheckerChange(Piece* oldCheckers[2][2], vector<Square>* savers[2]);
    ~CheckerChange() {}
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