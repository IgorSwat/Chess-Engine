#include "boardconfig.h"

namespace FenParsing {
    constexpr int blackID = 16;

    bool parseFenToConfig(const std::string& FEN, BoardConfig* config)
    {
        int row{ 0 }, col{ 0 };
        unsigned int i = 0;
        int whitePieces = 1;
        int blackPieces = 1;
        config->clearBoard();
        while (i < FEN.size() && FEN[i] != ' ')
        {
            switch (FEN[i])
            {
            case 'p':
                config->placePiece(blackID + blackPieces, PieceType::PAWN, Square(col, row), false);
                blackPieces++;
                col++;
                break;
            case 'n':
                config->placePiece(blackID + blackPieces, KNIGHT, Square(col, row),  false);
                blackPieces++;
                col++;
                break;
            case 'b':
                config->placePiece(blackID + blackPieces, BISHOP, Square(col, row), false);
                blackPieces++;
                col++;
                break;
            case 'r':
                config->placePiece(blackID + blackPieces, ROOK, Square(col, row), false);
                blackPieces++;
                col++;
                break;
            case 'q':
                config->placePiece(blackID + blackPieces, QUEEN, Square(col, row), false);
                blackPieces++;
                col++;
                break;
            case 'k':
                config->placePiece(blackID, KING, Square(col, row), false);
                col++;
                break;
            case 'P':
                config->placePiece(whitePieces, PieceType::PAWN, Square(col, row), false);
                whitePieces++;
                col++;
                break;
            case 'N':
                config->placePiece(whitePieces, KNIGHT, Square(col, row), false);
                whitePieces++;
                col++;
                break;
            case 'B':
                config->placePiece(whitePieces, BISHOP, Square(col, row), false);
                whitePieces++;
                col++;
                break;
            case 'R':
                config->placePiece(whitePieces, ROOK, Square(col, row), false);
                whitePieces++;
                col++;
                break;
            case 'Q':
                config->placePiece(whitePieces, QUEEN, Square(col, row), false);
                whitePieces++;
                col++;
                break;
            case 'K':
                config->placePiece(0, KING, Square(col, row), false);
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
        std::bitset<4> castlingRights;
        while (i < FEN.size() && FEN[i] != ' ')
        {
            switch (FEN[i])
            {
            case 'k':
                castlingRights[2] = true;
                break;
            case 'q':
                castlingRights[3] = true;
                break;
            case 'K':
                castlingRights[0] = true;
                break;
            case 'Q':
                castlingRights[1] = true;
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
        config->castlingRights = castlingRights;
        return true;
    }
}
