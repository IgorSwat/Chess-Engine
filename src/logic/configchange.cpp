#include "boardconfig.h"

CheckerChange::CheckerChange(Piece* oldCheckers[2][2], vector<Square>* savers[2])
{
    for (int i = 0; i < 2; i++)
    {
        checkSavers[i] = savers[i];
        for (int j = 0; j < 2; j++)
            checkers[i][j] = oldCheckers[i][j];
    }
}

void PlacementChange::applyChange(BoardConfig* config) const
{
    PieceType type = config->getPiece(pieceID)->getType();
    config->placePiece(pieceID, type, position);
}

void CastlingChange::applyChange(BoardConfig* config) const
{
    config->setCastlingRight(WHITE, SHORT_CASTLE, castlingRights[0]);
    config->setCastlingRight(WHITE, LONG_CASTLE, castlingRights[1]);
    config->setCastlingRight(BLACK, SHORT_CASTLE, castlingRights[2]);
    config->setCastlingRight(BLACK, LONG_CASTLE, castlingRights[3]);
}

void EnPassantChange::applyChange(BoardConfig* config) const
{
    config->setEnPassantLine(enPassantRow);
    config->enPassanter1 = this->enPassanter1;
    config->enPassanter2 = this->enPassanter2;
}

void CheckerChange::applyChange(BoardConfig* config) const
{
    for (int i = 0; i < 2; i++)
    {
        config->safeMoves[i] = checkSavers[i];
        for (int j = 0; j < 2; j++)
            config->checkers[i][j] = checkers[i][j];
    }
}

void PromotionChange::applyChange(BoardConfig* config) const
{
    Square pos = config->getPiece(pieceID)->getPosition();
    config->placePiece(pieceID, PieceType::PAWN, pos, false);
}

void MoveCountChange::applyChange(BoardConfig* config) const
{
    config->halfMoves = halfMoves;
    config->moveNumber = moveNumber;
}

