#include "pgnParser.h"
#include <cctype>
#include <algorithm>


// ----------------------------
// Helper functions and defines
// ----------------------------

namespace {

    constexpr bool isOpeningBracket(char c) {return c == '(' || c == '[' || c == '{';}
    constexpr bool isClosingBracket(char c) {return c == ')' || c == ']' || c == '}';}
    constexpr bool isNotationEnd(char c) {return c == ' ' || c == '#' || c == '+' || c == '!' || c == '?';}

}


// -----------------
// PgnParser methods
// -----------------

PgnParser::PgnParser(const std::string& pgnFilePath, BoardConfig* board)
    : pgn(Utilities::readFile(pgnFilePath)), currentPos(pgn.begin()), board(board)
{
}

bool PgnParser::processNext()
{
    if (!parseToNextMove())
        return false;
    std::string notation = parseMove();

    Move move = moveFromNotation(notation);
    if (move == NULL_MOVE)
        return false;
    
    board->makeMove(move);
    return true;
}

void PgnParser::processAll()
{
    while (processNext()) {}
}

void PgnParser::reset()
{
    currentPos = pgn.begin();
    bracketCount = 0;
}

bool PgnParser::parseToNextMove()
{
    while (currentPos != pgn.end()) {
        char c = *currentPos;
        if (isalpha(c) && bracketCount == 0)
            return true;
        if (isOpeningBracket(c))
            bracketCount++;
        else if (isClosingBracket(c))
            bracketCount--;
        currentPos++;
    }
    return false;
}

std::string PgnParser::parseMove()
{
    std::string::iterator notationEnd = std::find_if(currentPos, pgn.end(), isNotationEnd);
    std::string result = notationEnd != pgn.end() ? 
                         pgn.substr(std::distance(pgn.begin(), currentPos), std::distance(currentPos, notationEnd)) :
                         "";
    currentPos = notationEnd;
    return result;
}

Move PgnParser::moveFromNotation(const std::string& notation) const
{
    Movemask mask = QUIET_MOVE_FLAG;
    Square from, to;

    // Castling
    if (notation[0] == 'O') {
        from = board->movingSide() == WHITE ? SQ_E1 : SQ_E8;
        to = board->movingSide() == WHITE ?
                (notation.size() == 3 ? SQ_G1 : SQ_C1) :
                (notation.size() == 3 ? SQ_G8 : SQ_C8);
        mask = notation.size() == 3 ? KINGSIDE_CASTLE_FLAG : QUEENSIDE_CASTLE_FLAG;
        return Move(from, to, mask);
    }
    // Pawn move
    else if (std::islower(notation[0])) {
        Bitboard file = Board::Files[notation[0] - 'a'];

        // Capture
        if (notation[1] == 'x') {
            to = squareFromNotation(notation.substr(2, 2));
            from = Bitboards::lsb(Pieces::pawn_attacks(~board->movingSide(), to) & file);
            mask |= CAPTURE_FLAG;
            // En passant check
            if (board->onSquare(to) == NO_PIECE)
                mask |= ENPASSANT_FLAG;
        }
        else {
            Direction forwardDir = board->movingSide() == WHITE ? NORTH : SOUTH;
            to = squareFromNotation(notation.substr(0, 2));
            // 2 squares pawn move
            if (board->onSquare(to - forwardDir) == NO_PIECE) {
                from = to - forwardDir - forwardDir;
                mask |= DOUBLE_PAWN_PUSH_FLAG;
            }
            else
                from = to - forwardDir;
        }

        // Promotion
        if (std::find(notation.begin(), notation.end(), '=') != notation.end()) {
            Movemask promotionFlag = notation.back() == 'Q' ? QUEEN_PROMOTION_FLAG :
                                     notation.back() == 'R' ? ROOK_PROMOTION_FLAG :
                                     notation.back() == 'N' ? KNIGHT_PROMOTION_FLAG : 
                                                              BISHOP_PROMOTION_FLAG;
            mask |= promotionFlag;
        }
    }
    // Piece move
    else if (std::isupper(notation[0])) {
        PieceType ptype = notation[0] == 'N' ? KNIGHT :
                          notation[0] == 'B' ? BISHOP :
                          notation[0] == 'R' ? ROOK :
                          notation[0] == 'Q' ? QUEEN : KING;
        Bitboard fromArea = board->pieces(board->movingSide(), ptype);
        bool isCapture = std::find(notation.begin(), notation.end(), 'x') != notation.end();
        
        // Capture
        if (isCapture)
            mask |= CAPTURE_FLAG;
        // Singly disambiguated move
        if (!isCapture && notation.size() == 4 || isCapture && notation.size() == 5)
            fromArea &= isalpha(notation[1]) ? Board::Files[notation[1] - 'a'] :
                                               Board::Ranks[notation[1] - '1'];
        // Doubly disambiguated move
        else if (!isCapture && notation.size() == 5 || isCapture && notation.size() == 6)
            fromArea &= square_to_bb(squareFromNotation(notation.substr(1, 2)));

        to = squareFromNotation(notation.substr(notation.size() - 2, 2));
        from = Bitboards::lsb(Pieces::piece_attacks_d(ptype, to, board->pieces()) & fromArea);
    }
    // Invalid notation
    else
        return NULL_MOVE;

    return Move(from, to, mask);
}

Square PgnParser::squareFromNotation(const std::string& notation)
{
    int rank = notation[1] - '1';
    int file = notation[0] - 'a';
    return make_square(rank, file);
}