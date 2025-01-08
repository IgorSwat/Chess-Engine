#include "parsing.h"


namespace Utilities::Parsing {

    // ----------------------
    // Primitive type parsing
    // ----------------------

    Square parse_square(const std::string& notation)
    {
        int rank = notation[1] - '1';
        int file = notation[0] - 'a';

        return make_square(rank, file);
    }

    // ------------
    // Move parsing
    // ------------

    Move parse_move(const BoardConfig& board, const std::string& notation)
    {
        Movemask mask = QUIET_MOVE_FLAG;
        Square from, to;

        // Castling
        if (notation[0] == 'O') {
            from = board.movingSide() == WHITE ? SQ_E1 : SQ_E8;
            to = board.movingSide() == WHITE ?
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
                to = parse_square(notation.substr(2, 2));
                from = Bitboards::lsb(Pieces::pawn_attacks(~board.movingSide(), to) & file);
                mask |= CAPTURE_FLAG;
                // En passant check
                if (board.onSquare(to) == NO_PIECE)
                    mask |= ENPASSANT_FLAG;
            }
            else {
                Direction forwardDir = board.movingSide() == WHITE ? NORTH : SOUTH;
                to = parse_square(notation.substr(0, 2));
                // 2 squares pawn move
                if (board.onSquare(to - forwardDir) == NO_PIECE) {
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
            Bitboard fromArea = board.pieces(board.movingSide(), ptype);
            bool isCapture = std::find(notation.begin(), notation.end(), 'x') != notation.end();
        
            // Capture
            if (isCapture)
                mask |= CAPTURE_FLAG;

            std::size_t notationSize = notation.back() == '+' ? notation.size() - 1 : notation.size();

            // Single disambiguated move
            if (!isCapture && notationSize == 4 || isCapture && notationSize == 5)
                fromArea &= isalpha(notation[1]) ? Board::Files[notation[1] - 'a'] :
                                                   Board::Ranks[notation[1] - '1'];
            // Double disambiguated move
            else if (!isCapture && notationSize == 5 || isCapture && notationSize == 6)
                fromArea &= square_to_bb(parse_square(notation.substr(1, 2)));

            to = parse_square(notation.substr(notationSize - 2, 2));
            from = Bitboards::lsb(Pieces::piece_attacks_d(ptype, to, board.pieces()) & fromArea);
        }

        // Invalid notation
        else
            return Move::null();

        return Move(from, to, mask);
    }

}