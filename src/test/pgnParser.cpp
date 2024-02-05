#include "pgnParser.h"
#include "../logic/boardConfig.h"
#include <cctype>
#include <cmath>
#include <fstream>
#include <exception>


namespace {

    Square readPosition(char fileSymbol, char rankSymbol)
    {
        Bitboard file = Board::Files[fileSymbol - 'a'];
        Bitboard rank = Board::Ranks[rankSymbol - '1'];
        return Bitboards::lsb(file & rank);
    }

}


void PGNParser::loadPGN(const std::string& filepath)
{
    std::ifstream file;

    file.open(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Unable to open file " + filepath);

    std::streamsize fileSize = file.tellg();
    std::string pgn(fileSize, ' ');

    file.seekg(0, std::ios::beg);
    file.read(&pgn[0], fileSize);
    file.close();

    input.clear();
    input.str(pgn);
}

bool PGNParser::applyNextMove()
{
    std::string moveNotation = "";

    if (!parseUntilNextMove() || (moveNotation = parseNextMove()) == "")
        return false;
    Move move = moveFromNotation(moveNotation);
    board->makeMove(move);
    return true;
}

bool PGNParser::parseUntilNextMove()
{
    int bracketCount = 0;

    while (!input.eof() && (!isdigit(input.peek()) || bracketCount > 0)) {
        char c = input.get();
        if (c == '(' || c == '[' || c == '{')
            bracketCount++;
        else if (c == ')' || c == ']' || c == '}')
            bracketCount--;
    }

    return !input.eof();
}

std::string PGNParser::parseNextMove()
{
    std::string moveNotation = "";
    char c;

    while (!input.eof() && !isalpha(input.peek()))
        input.get();
    while (!input.eof() && (c = input.get()) != ' ')
        moveNotation += c;

    return moveNotation;
}

Move PGNParser::moveFromNotation(const std::string& moveNotation) const
{
    char pieceSymbol = moveNotation[0];
    Bitboard mask = 0xffffffffffffffff;
    Color sideOnMove = board->movingSide();
    Movemask flags = QUIET_MOVE_FLAG;

    if (isupper(pieceSymbol)) {     // Piece move
        if (pieceSymbol == 'O')
            return castleFromNotation(moveNotation);

        PieceType pieceType = pieceSymbol == 'N' ? KNIGHT :
                              pieceSymbol == 'B' ? BISHOP :
                              pieceSymbol == 'R' ? ROOK :
                              pieceSymbol == 'Q' ? QUEEN : KING;
        int targetId = 1;

        if (moveNotation[targetId] != 'x' && (isdigit(moveNotation[targetId]) || isalpha(moveNotation[targetId + 1]))) {
            char specifier = moveNotation[targetId];
            Bitboard specifiedArea = isalpha(specifier) ? Board::Files[specifier - 'a'] : Board::Ranks[specifier - '1'];
            mask &= specifiedArea;
            targetId++;
        }

        if (moveNotation[targetId] == 'x') {
            flags |= CAPTURE_FLAG;
            targetId++;
        }
        
        Square to = readPosition(moveNotation[targetId], moveNotation[targetId + 1]);
        mask &= Pieces::piece_attacks_d(pieceType, to, board->pieces());
        Square from = Bitboards::lsb(mask & board->pieces(sideOnMove, pieceType));

        return Move(from, to, flags);
    }
    else {      // Pawn move
        Square from, to;
        Movemask flags = QUIET_MOVE_FLAG;
        mask &= Board::Files[moveNotation[0] - 'a'];

        int sep = moveNotation.find('=');
        if (sep != std::string::npos) {     // Promotion
            char promotionSymbol = moveNotation[sep + 1];
            Movemask promotionFlag = promotionSymbol == 'Q' ? QUEEN_PROMOTION_FLAG :
                                     promotionSymbol == 'R' ? ROOK_PROMOTION_FLAG :
                                     promotionSymbol == 'B' ? BISHOP_PROMOTION_FLAG : KNIGHT_PROMOTION_FLAG;
            flags |= promotionFlag;
        }

        if (moveNotation[1] == 'x') {       // Capture
            flags |= CAPTURE_FLAG;
            to = readPosition(moveNotation[2], moveNotation[3]);
            if (board->onSquare(to) == NO_PIECE)
                flags = ENPASSANT_FLAG;
            mask &= Pieces::PAWN_ATTACKS[~sideOnMove][to];
            from = Bitboards::lsb(board->pieces(sideOnMove, PAWN) & mask);
        }
        else {
            to = readPosition(moveNotation[0], moveNotation[1]);
            from = sideOnMove == WHITE ? (board->onSquare(to + SOUTH) != NO_PIECE ? to + SOUTH : to + SOUTH + SOUTH) :
                                         (board->onSquare(to + NORTH) != NO_PIECE ? to + NORTH : to + NORTH + NORTH);
            flags = sideOnMove == WHITE ? (board->onSquare(to + SOUTH) != NO_PIECE ? flags : flags | DOUBLE_PAWN_PUSH_FLAG) :
                                          (board->onSquare(to + NORTH) != NO_PIECE ? flags : flags | DOUBLE_PAWN_PUSH_FLAG);
        }

        return Move(from, to, flags);
    }
}

Move PGNParser::castleFromNotation(const std::string& moveNotation) const
{
    Square from = board->movingSide() == WHITE ? SQ_E1 : SQ_E8;
    Square to;
    Movemask flags;

    if (moveNotation == "O-O") {      // Short castle
        to = board->movingSide() == WHITE ? SQ_G1 : SQ_G8;
        flags = KINGSIDE_CASTLE_FLAG;
    }
    else {      // Long castle
        to = board->movingSide() == WHITE ? SQ_C1 : SQ_C8;
        flags = QUEENSIDE_CASTLE_FLAG;
    }

    return Move(from, to, flags);
}