#include "movegen.h"
#include <algorithm>


namespace MoveGeneration {

    // -----------------------------------------
    // Move generation - collective - pawn moves
    // -----------------------------------------

    // Helper function - promotion generator
    // - Since one pawn promotion can generate 4 different moves, we group it together in one helper function
    template <bool capture, typename MoveT>
	void generate_promotions(Square from, Square to, Moves::List<MoveT>& movelist)
	{
        using namespace Moves;

        // NOTE: To statically improve move ordering, we put more valuable promotions before others
		movelist.push_back(Move(from, to, capture ? CAPTURE_FLAG | QUEEN_PROMOTION_FLAG : QUEEN_PROMOTION_FLAG));
		movelist.push_back(Move(from, to, capture ? CAPTURE_FLAG | ROOK_PROMOTION_FLAG : ROOK_PROMOTION_FLAG));
		movelist.push_back(Move(from, to, capture ? CAPTURE_FLAG | BISHOP_PROMOTION_FLAG : BISHOP_PROMOTION_FLAG));
		movelist.push_back(Move(from, to, capture ? CAPTURE_FLAG | KNIGHT_PROMOTION_FLAG : KNIGHT_PROMOTION_FLAG));
	}

    template <Mode mode, Color side, typename MoveT>
    void generate_pawn_moves(const Board& board, Bitboard target, Moves::List<MoveT>& movelist)
    {
        // Compile time properties
        constexpr Color enemy = ~side;

        constexpr Bitboard third_rank = side == WHITE ? Chessboard::RANK_3 : Chessboard::RANK_6;
        constexpr Bitboard penultimate_rank = side == WHITE ? Chessboard::RANK_7 : Chessboard::RANK_2;
        constexpr Bitboard last_rank = side == WHITE ? Chessboard::RANK_8 : Chessboard::RANK_1;

        constexpr Direction forward = side == WHITE ? NORTH : SOUTH;
        constexpr Direction forward_left = side == WHITE ? NORTH_WEST : SOUTH_WEST;
        constexpr Direction forward_right = side == WHITE ? NORTH_EAST : SOUTH_EAST;

        // Runtime properties
        Bitboard pawns = board.pieces(side, PAWN);
        Bitboard pawns_on_7th = pawns & penultimate_rank;
        Bitboard pawns_not_on_7th = pawns & ~penultimate_rank;

        Bitboard empty_squares = ~board.pieces();
        Bitboard enemy_pieces = board.pieces(enemy);

        // Step 1 - adjust target map with respect to given generation mode
        // - Since CAPTURE mode includes pawn promotions, promotion squares (8th rank for given side) must be included in target set
        // - QUIET mode should not include checks, and QUIET_CHECK should include only checks
        if constexpr (mode == CAPTURE)
			target |= last_rank;
		if constexpr (mode == QUIET)
			target &= ~board.possible_checks(PAWN);
		if constexpr (mode == QUIET_CHECK)
			target &= board.possible_checks(PAWN);

        Bitboard quiet_target = target & empty_squares;
        Bitboard capture_target = target & enemy_pieces;

        // Step 2 - generate quiet moves (single and double pawn pushes)
        // - Not present in CAPTURE mode
        // - Ignore promotion pushes (since they are not considered quiet and covered only in CAPTURE mode)
        if constexpr (mode != CAPTURE) {
            Bitboard possible_pushes = Bitboards::shift_s<forward>(pawns_not_on_7th) & empty_squares;

            Bitboard single_pushes = possible_pushes & quiet_target;
            Bitboard double_pushes = Bitboards::shift_s<forward>(possible_pushes & third_rank) & quiet_target;

            // Extract single moves from move maps
			while (single_pushes) {
				Square to = Bitboards::pop_lsb(single_pushes);
				movelist.push_back(Move(to - forward, to, Moves::QUIET_MOVE_FLAG));
			}
			while (double_pushes) {
				Square to = Bitboards::pop_lsb(double_pushes);
				movelist.push_back(Move(to - forward - forward, to, Moves::DOUBLE_PAWN_PUSH_FLAG));
			}
		}

        // Step 3 - generate captures
        // - Not present in both QUIET and QUIET_CHECK modes
        // - Include enpassant since it is a capture
        // - Do not include captures with promotions - they have their own section
        if constexpr (mode != QUIET && mode != QUIET_CHECK) {
            // Common captures
			Bitboard left_captures = Bitboards::shift_s<forward_left>(pawns_not_on_7th) & capture_target;
			Bitboard right_captures = Bitboards::shift_s<forward_right>(pawns_not_on_7th) & capture_target;

            // Extract single moves from move maps
			while (left_captures) {
				Square to = Bitboards::pop_lsb(left_captures);
				movelist.push_back(Move(to - forward_left, to, Moves::CAPTURE_FLAG));
			}
			while (right_captures) {
				Square to = Bitboards::pop_lsb(right_captures);
				movelist.push_back(Move(to - forward_right, to, Moves::CAPTURE_FLAG));
			}

			// Enpassant
            // WARNING: original condition: gen != CHECK_EVASION || target & board.enpassantSquare()
			if (target & board.enpassant_square()) {
				Bitboard enpassant_candidates = pawns_not_on_7th & Chessboard::adjacent_rank_squares(board.enpassant_square());
				Square to = board.enpassant_square() + forward;

                // Extract single moves from move maps
				while (enpassant_candidates)
					movelist.push_back(Move(Bitboards::pop_lsb(enpassant_candidates), to, Moves::ENPASSANT_FLAG));
			}
        }

        // Step 4 - generate promotions
        // - Considered as captures because they affect material balance on the board
        // - Not present in both QUIET and QUIET_CHECK modes
        if constexpr (mode != QUIET && mode != QUIET_CHECK) {
			Bitboard quiet_promotions = Bitboards::shift_s<forward>(pawns_on_7th) & quiet_target;
			Bitboard left_captures = Bitboards::shift_s<forward_left>(pawns_on_7th) & capture_target;
			Bitboard right_captures = Bitboards::shift_s<forward_right>(pawns_on_7th) & capture_target;

            // Extract single moves from move maps
			while (quiet_promotions) {
				Square to = Bitboards::pop_lsb(quiet_promotions);
				generate_promotions<false>(to - forward, to, movelist);
			}
			while (left_captures) {
				Square to = Bitboards::pop_lsb(left_captures);
				generate_promotions<true>(to - forward_left, to, movelist);
			}
			while (right_captures) {
				Square to = Bitboards::pop_lsb(right_captures);
				generate_promotions<true>(to - forward_right, to, movelist);
			}
		}
    }


    // -----------------------------------------
    // Move generation - collective - king moves
    // -----------------------------------------

    template <Mode mode, Color side, typename MoveT>
    void generate_king_moves(const Board& board, Bitboard target, Moves::List<MoveT>& movelist)
    {
        // There can be only 1 king of given color on the board, so we can determine from square
        Square from = board.king_position(side);

        Bitboard possible_moves = Pieces::piece_attacks_s<KING>(from) & target;

        // Step 1 - generate standard king moves
        while (possible_moves) {
            Square to = Bitboards::pop_lsb(possible_moves);

            // Since target might contain both quiet and capture squares (for example - CHECK_EVASION mode), 
            // we must determine which one is quiet and which one is capture
            // - NOTE: there is no need to check whether board.on(to) is friendly piece, because it's already done by applying target map
            movelist.push_back(Move(from, to, board.occupied(to) ? Moves::CAPTURE_FLAG : Moves::QUIET_MOVE_FLAG));
        }

        // Step 2 - generate castling
        // - Not present in both CAPTURE (since castle is always a quiet move) and CHECK_EVASION (since castle cannot be played when being in check)
        if constexpr (mode != CAPTURE && mode != CHECK_EVASION) {
			if (board.can_castle(side, KINGSIDE_CASTLE) && board.castle_path_clear(side, KINGSIDE_CASTLE))
				movelist.push_back(Move(from, Square(from + EAST + EAST), Moves::KINGSIDE_CASTLE_FLAG));
			if (board.can_castle(side, QUEENSIDE_CASTLE) && board.castle_path_clear(side, QUEENSIDE_CASTLE))
				movelist.push_back(Move(from, Square(from + WEST + WEST), Moves::QUEENSIDE_CASTLE_FLAG));
        }
    }


    // ------------------------------------------------
    // Move generation - collective - other piece moves
    // ------------------------------------------------

    // Knight, bishop, rook and queen moves are easy to generate, since there are no special moves related to those piece types
    template <Mode mode, Color side, PieceType ptype, typename MoveT>
    void generate_piece_moves(const Board& board, Bitboard target, Moves::List<MoveT>& movelist)
    {
        Bitboard pieces = board.pieces(side, ptype);

        // Step 1 - adjust target map with respect to given generation mode
        // - QUIET mode should not include checks, and QUIET_CHECK should include only checks
        if constexpr (mode == QUIET)
			target &= ~board.possible_checks(ptype);
		if constexpr (mode == QUIET_CHECK)
			target &= board.possible_checks(ptype);

        // Step 2 - generate all moves
        // - Since there might be multiple copies of each piece (like two knights or two rooks), we need additional loop
        while (pieces) {
            Square from = Bitboards::pop_lsb(pieces);

            Bitboard possible_moves = Pieces::piece_attacks_s<ptype>(from, board.pieces()) & target;

            // Since target might contain both quiet and capture squares (for example - CHECK_EVASION mode), 
            // we must determine which one is quiet and which one is capture
            // - NOTE: there is no need to check whether board.on(to) is friendly piece, because it's already done by applying target map
            while (possible_moves) {
                Square to = Bitboards::pop_lsb(possible_moves);

                movelist.push_back(Move(from, to, board.occupied(to) ? Moves::CAPTURE_FLAG : Moves::QUIET_MOVE_FLAG));
            }
        }
    }


    // -----------------------------------------
    // Move generation - collective - side moves
    // -----------------------------------------

    template <Mode mode, Color side, typename MoveT>
    void generate_side_moves(const Board& board, Moves::List<MoveT>& movelist)
    {
        // Compile time properties
        constexpr Color enemy = ~side;

        // Step 1 - calculate target map depending on generation mode
        // - NOTE: we do not cover check / no check property here, so the target must be updated in specialized generators (for piece types)
        Bitboard target = mode == CAPTURE     ? board.pieces(enemy) :
                          mode == QUIET_CHECK ? ~board.pieces() :
                          mode == QUIET       ? ~board.pieces() :
                                               ~board.pieces(side);

        // Step 2 - check evasions
        // - CHECK_EVASION requires updating target map to contain only moves that deal with check
        if constexpr (mode == CHECK_EVASION) {
            // Single checks
            if (Bitboards::singly_populated(board.checkers())) {
                Square checker_pos = Bitboards::lsb(board.checkers());

                Bitboard evasion_target = target & (Chessboard::Paths[board.king_position(side)][checker_pos] | checker_pos);

                // - For non-king pieces, generate only moves that blocks check or captures checking piece (evasion_target)
                // - For king, generate moves as usual (just without castle)
                generate_pawn_moves<CHECK_EVASION, side>(board, evasion_target, movelist);
				generate_piece_moves<CHECK_EVASION, side, KNIGHT>(board, evasion_target, movelist);
				generate_piece_moves<CHECK_EVASION, side, BISHOP>(board, evasion_target, movelist);
				generate_piece_moves<CHECK_EVASION, side, ROOK>(board, evasion_target, movelist);
				generate_piece_moves<CHECK_EVASION, side, QUEEN>(board, evasion_target, movelist);
				generate_king_moves<CHECK_EVASION, side>(board, target, movelist);
            }
            // Double checks
            // - Only king moves can be pseudolegal in double check situations
            else
                generate_king_moves<CHECK_EVASION, side>(board, target, movelist);
        }
        // Step 3 - other modes
        else {
            generate_pawn_moves<mode, side>(board, target, movelist);
			generate_piece_moves<mode, side, KNIGHT>(board, target, movelist);
			generate_piece_moves<mode, side, BISHOP>(board, target, movelist);
			generate_piece_moves<mode, side, ROOK>(board, target, movelist);
			generate_piece_moves<mode, side, QUEEN>(board, target, movelist);

            // King moves can never be checks, so we can omit them in QUIET_CHECK case
			if constexpr (mode != QUIET_CHECK)
				generate_king_moves<mode, side>(board, target, movelist);
        }
    }


    // ----------------------------
    // Move generation - collective
    // ----------------------------

    // Main move generation function - library's API
    // - Generate moves for current side to move
    template <Mode mode, typename MoveT>
    void generate_moves(const Board& board, Moves::List<MoveT>& movelist)
    {
        if (board.side_to_move() == WHITE)
            generate_side_moves<mode, WHITE>(board, movelist);
        else
            generate_side_moves<mode, BLACK>(board, movelist);
    }

    // Usages declaration
    template void generate_moves<QUIET, Move>(const Board&, Moves::List<Move>&);
	template void generate_moves<CAPTURE, Move>(const Board&, Moves::List<Move>&);
	template void generate_moves<QUIET_CHECK, Move>(const Board&, Moves::List<Move>&);
	template void generate_moves<CHECK_EVASION, Move>(const Board&, Moves::List<Move>&);
	template void generate_moves<PSEUDO_LEGAL, Move>(const Board&, Moves::List<Move>&);
    template void generate_moves<QUIET, EMove>(const Board&, Moves::List<EMove>&);
	template void generate_moves<CAPTURE, EMove>(const Board&, Moves::List<EMove>&);
	template void generate_moves<QUIET_CHECK, EMove>(const Board&, Moves::List<EMove>&);
	template void generate_moves<CHECK_EVASION, EMove>(const Board&, Moves::List<EMove>&);
	template void generate_moves<PSEUDO_LEGAL, EMove>(const Board&, Moves::List<EMove>&);

    // Specialized version - legal moves generation
    // - Legal move generation is basically the same as pseudolegal, with an additional check with board.is_legal_p()
    // - NOTE: since we basically never use full legal generation in search mechanism, we do not need to implement it's EnhancedMove version
    template <>
    void generate_moves<LEGAL, Move>(const Board& board, Moves::List<Move>& movelist)
    {
        if (board.in_check())
			generate_moves<CHECK_EVASION>(board, movelist);
		else
			generate_moves<PSEUDO_LEGAL>(board, movelist);

        // Test each move with board.is_legal_p() and put every legal move in front of the list
        Move* end_of_legals = std::partition(movelist.begin(), movelist.end(), [&board](const Move& move) {return board.is_legal_p(move);});
		movelist.set_end(end_of_legals);
    }


    // ----------------------------
    // Move generation - individual
    // ----------------------------

    Move create_move(const Board& board, Square from, Square to)
	{
		Moves::Mask mask = 0;

		Piece piece = board.on(from);
		if (piece == NO_PIECE)
			return Moves::null;

		// During creation of move, we make assumption that move is fully correct, to make minimum amount of checks for given flag.
		// Obviously move might be completely illegal, so it needs to be checked with BoardConfig.legalityTestFull()
		if (board.on(to) != NO_PIECE && color_of(board.on(to)) != color_of(piece))
			mask |= Moves::CAPTURE_FLAG;
		if (type_of(piece) == PAWN) {
			Direction forward = color_of(piece) == WHITE ? NORTH : SOUTH;

			if (file_of(from) != file_of(to) && to == board.enpassant_square() + forward)
				mask |= Moves::ENPASSANT_FLAG;
			else if ((rank_of(to) - rank_of(from)) % 2 == 0)
				mask |= Moves::DOUBLE_PAWN_PUSH_FLAG;
			// Very important - we always detect promotion
			else if (rank_of(to) == 0 || rank_of(to) == 7)
				mask |= Moves::PROMOTION_FLAG;
		}
		if (type_of(piece) == KING && file_of(from) == FILE_E) {
			if (to == (from + EAST + EAST))
				mask |= Moves::KINGSIDE_CASTLE_FLAG;
			else if (to == (from + WEST + WEST))
				mask |= Moves::QUEENSIDE_CASTLE_FLAG;
		}

		return Move(from, to, mask);
	}
}