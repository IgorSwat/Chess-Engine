#include "boardconfig.h"

CheckerChange::CheckerChange(Piece** wCheckers, Piece** bCheckers, std::vector<Square>** savers)
{
    this->whiteCheckers = new Piece* [2];
    this->blackCheckers = new Piece* [2];
    this->checkSavers = new std::vector<Square>* [2];
    this->whiteCheckers[0] = wCheckers[0];
    this->whiteCheckers[1] = wCheckers[1];
    this->blackCheckers[0] = bCheckers[0];
    this->blackCheckers[1] = bCheckers[1];
    this->checkSavers[0] = savers[0];
    this->checkSavers[1] = savers[1];
}

void PlacementChange::applyChange(BoardConfig* config) const
{
    config->placePiece(this->position, this->pieceID);
}

void CastlingChange::applyChange(BoardConfig* config) const
{
    config->Wshort = this->Wshort;
    config->Wlong = this->Wlong;
    config->Bshort = this->Bshort;
    config->Blong = this->Blong;
}

void EnPassantChange::applyChange(BoardConfig* config) const
{
    config->enPassantLine = this->enPassantRow;
    config->enPassanter1 = this->enPassanter1;
    config->enPassanter2 = this->enPassanter2;
}

void SideOnMoveChange::applyChange(BoardConfig* config) const
{
    config->sideOnMove = this->sideOnMove;
}

void CheckerChange::applyChange(BoardConfig* config) const
{
    config->whiteCkecker[0] = this->whiteCheckers[0];
    config->whiteCkecker[1] = this->whiteCheckers[1];
    config->blackCkecker[0] = this->blackCheckers[0];
    config->blackCkecker[1] = this->blackCheckers[1];
    config->safeMoves[0] = this->checkSavers[0];
    config->safeMoves[1] = this->checkSavers[1];
}

void PromotionChange::applyChange(BoardConfig* config) const
{
    Square pos = config->getPiece(pieceID)->getPos();
    config->placePiece(pos, pieceID, PieceType::PAWN, false);
}

void MoveCountChange::applyChange(BoardConfig* config) const
{
    config->halfMoves = halfMoves;
    config->moveNumber = moveNumber;
}

