#include "boardconfig.h"

bool FenReader::parseToConfig(BoardConfig* config) const
{
    int row {0}, col {0};
    unsigned int i = 0;
    int whitePieces = 1;
    int blackPieces = 1;
    config->clearBoard();
    while (i < FEN.size() && FEN[i] != ' ')
    {
        switch (FEN[i])
        {
        case 'p':
            config->placePiece(Square(col, row), BoardConfig::blackID + blackPieces, PieceType::PAWN, false);
            blackPieces++;
            col++;
            break;
        case 'n':
            config->placePiece(Square(col, row), BoardConfig::blackID + blackPieces, KNIGHT, false);
            blackPieces++;
            col++;
            break;
        case 'b':
            config->placePiece(Square(col, row), BoardConfig::blackID + blackPieces, BISHOP, false);
            blackPieces++;
            col++;
            break;
        case 'r':
            config->placePiece(Square(col, row), BoardConfig::blackID + blackPieces, ROOK, false);
            blackPieces++;
            col++;
            break;
        case 'q':
            config->placePiece(Square(col, row), BoardConfig::blackID + blackPieces, QUEEN, false);
            blackPieces++;
            col++;
            break;
        case 'k':
            config->placePiece(Square(col, row), BoardConfig::blackID, false);
            col++;
            break;
        case 'P':
            config->placePiece(Square(col, row), whitePieces, PieceType::PAWN, false);
            whitePieces++;
            col++;
            break;
        case 'N':
            config->placePiece(Square(col, row), whitePieces, KNIGHT, false);
            whitePieces++;
            col++;
            break;
        case 'B':
            config->placePiece(Square(col, row), whitePieces, BISHOP, false);
            whitePieces++;
            col++;
            break;
        case 'R':
            config->placePiece(Square(col, row), whitePieces, ROOK, false);
            whitePieces++;
            col++;
            break;
        case 'Q':
            config->placePiece(Square(col, row), whitePieces, QUEEN, false);
            whitePieces++;
            col++;
            break;
        case 'K':
            config->placePiece(Square(col, row), 0, false);
            col++;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            col += static_cast<int>(FEN[i] - '0');
            break;
        case '/':
            row += 1;
            col = 0;
            break;
        default:
            return false;
        }
        i++;
    }
    i++;
    switch (FEN[i])
    {
    case 'w':
        config->sideOnMove = WHITE;
        break;
    case 'b':
        config->sideOnMove = BLACK;
        break;
    default:
        return false;
    }
    i += 2;
    bool bshort {false}, blong {false}, wshort {false}, wlong {false};
    while (i < FEN.size() && FEN[i] != ' ')
    {
        switch (FEN[i])
        {
        case 'k':
            bshort = true;
            break;
        case 'q':
            blong = true;
            break;
        case 'K':
            wshort = true;
            break;
        case 'Q':
            wlong = true;
            break;
        case '-':
            break;
        default:
            return false;
        }
        i++;
    }
    i++;
    while (i < FEN.size() && FEN[i] != ' ')
    {
        switch (FEN[i])
        {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
            config->enPassantLine = static_cast<int>(FEN[i] - 'a');
            i++;
            break;
        case '-':
            break;
        default:
            return false;
        }
        i++;
    }
    i++;
    std::string val1, val2;
    while (i < FEN.size() && FEN[i] != ' ')
    {
        if (FEN[i] < '0' || FEN[i] > '9')
            return false;
        val1 += FEN[i];
        i++;
    }
    i++;
    while (i < FEN.size() && FEN[i] != ' ')
    {
        if (FEN[i] < '0' || FEN[i] > '9')
            return false;
        val2 += FEN[i];
        i++;
    }
    if (val1.size() == 0 || val2.size() == 0)
        return false;
    config->halfMoves = std::stoi(val1);
    config->moveNumber = std::stoi(val2);
    config->Wshort = wshort;
    config->Wlong = wlong;
    config->Bshort = bshort;
    config->Blong = blong;
    return true;
}

