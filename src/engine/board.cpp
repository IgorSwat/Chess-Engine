#include "board.h"
#include "evalconfig.h"
#include <algorithm>
#include <cstring>
#include <sstream>


namespace Chessboard {

    // --------------------------------
    // Board - position change - static
    // --------------------------------

    void Board::clear()
    {
        // Clear all the piece tables
        std::fill(m_board, m_board + SQUARE_RANGE, NO_PIECE);
        std::fill(m_pieces_c, m_pieces_c + COLOR_RANGE, 0);
        std::fill(m_pieces_t, m_pieces_t + PIECE_TYPE_RANGE, 0);
        std::fill(m_kings, m_kings + COLOR_RANGE, NULL_SQUARE);

        // Shrink position stack to ply 0 only and reset the data
        m_pstack.shrink();
        m_pstack.top() = Position();    // Default constructor for Position creates an empty board data
    }

    void Board::load_position(const std::string& fen)
    {
        // Let's start with reseting current board state
        clear();

        // FEN parsing with std::istringstream
	    std::istringstream stream(fen);
        std::string part;

        // Part 1 - parse piece placement
        stream >> part;
        int rank = 7, file = 0;		// In FEN we start from upper side of a board

        for (char symbol : part) {
            // New rank mark
            if (symbol == '/') {
                rank--;		// Go to another rank
                file = 0;   // Reset file id
            }
            // Shift inside the current rank
            else if (std::isdigit(symbol))
			    file += static_cast<int>(symbol - '0');
            // Piece mark
            else {
                Square square = make_square(Rank(rank), File(file));
                Color color = std::isupper(symbol) ? WHITE : BLACK;
                PieceType type = std::tolower(symbol) == 'p' ? PAWN :
                                 std::tolower(symbol) == 'n' ? KNIGHT :
                                 std::tolower(symbol) == 'b' ? BISHOP :
                                 std::tolower(symbol) == 'r' ? ROOK :
                                 std::tolower(symbol) == 'q' ? QUEEN : KING;

                place_piece(make_piece(color, type), square);

                // TODO: uncomment after implementing evaluation
                // posInfo->gameStageValue += Evaluation::PieceStageInfluence[make_piece(color, type)];

                file++;
            }
        }

        // Part 2 - side to move parsing
	    stream >> part;
	    m_moving_side = part == "w" ? WHITE : BLACK;

        // Part 3 - castling rights parsing
	    stream >> part;
	    if (part.find('K') != std::string::npos)
		    m_pstack.top().castling_rights |= make_castle_right(WHITE, KINGSIDE_CASTLE);
	    if (part.find('Q') != std::string::npos)
            m_pstack.top().castling_rights |= make_castle_right(WHITE, QUEENSIDE_CASTLE);
	    if (part.find('k') != std::string::npos)
            m_pstack.top().castling_rights |= make_castle_right(BLACK, KINGSIDE_CASTLE);
	    if (part.find('q') != std::string::npos)
            m_pstack.top().castling_rights |= make_castle_right(BLACK, QUEENSIDE_CASTLE);
        
        // Part 4 - enpassant parsing
	    stream >> part;
	    if (part != "-") {
		    Rank epRank = m_moving_side == WHITE ? ::RANK_5 : ::RANK_4;
		    File epFile = File(part.front() - 'a');

		    m_pstack.top().enpassant_square = make_square(epRank, epFile);
	    }

        // Part 5 - move counters
	    stream >> m_pstack.top().halfmove_clock;
	    stream >> m_halfmoves;
	    m_halfmoves = m_halfmoves * 2 + m_moving_side - 2;      // A reversed formula to moves_p()

        // Checks & pins update
        update_checks();
        update_pins(WHITE);
        update_pins(BLACK);

        // Zobrist hash update (static)
        m_zobrist.generate(*this);
        m_pstack.top().hash = m_zobrist.hash();
    }

    void Board::load_position(const Board& other)
    {
        // Copy common data
        m_moving_side = other.m_moving_side;
        m_halfmoves = other.m_halfmoves;
        std::copy(other.m_board, other.m_board + SQUARE_RANGE, m_board);
        std::copy(other.m_pieces_c, other.m_pieces_c + COLOR_RANGE, m_pieces_c);
	    std::copy(other.m_pieces_t, other.m_pieces_t + PIECE_TYPE_RANGE, m_pieces_t);
	    std::copy(other.m_kings, other.m_kings + COLOR_RANGE, m_kings);

        // Reset the stack and simply copy position data from other
        m_pstack.shrink();
        m_pstack.top() = other.m_pstack.top();

        // Clear fields related to last move (since we do not keep track of previous positions from other board)
        m_pstack.top().last_move = Moves::null;
        m_pstack.top().captured = NO_PIECE;

        // Load zobrist hash
        m_zobrist.set(m_pstack.top().hash);
    }


    // ---------------------------------------------------------------------
    // Board - position change - dynamic (move make & unmake) - normal moves
    // ---------------------------------------------------------------------

    // Main move maker function
    void Board::make_move(const Move& move)
    {
        // Push position stack to make room for new ply data
        m_pstack.push();

        m_pstack.top().last_move = move;

        // Delegate further updates to specialized function for each move type
        switch (move.type()) {
            case Moves::NORMAL:
                make_normal(move);
                break;
            case Moves::PROMOTION:
                make_promotion(move);
                break;
            case Moves::ENPASSANT:
                make_enpassant(move);
                break;
            case Moves::CASTLE:
                make_castle(move);
                break;
            default:
                return;
        }

        // Side to move change
        m_moving_side = ~m_moving_side;
        m_zobrist.update(m_moving_side);

        // Check & pin update - common for all types of moves
        // - NOTE: It's important to update side to move before update_checks() call
        update_checks();
	    update_pins(WHITE);
	    update_pins(BLACK);

        // Halfmove counter update - common for all types of moves
        m_halfmoves++;

        // Hash update - common for all types of moves
        m_pstack.top().hash = m_zobrist.hash();

    }

    // Specialized move makers - normal moves
    void Board::make_normal(const Move& move)
    {
        Square from = move.from();
        Square to = move.to();
        Piece piece = m_board[from];

        // Step 1 - update piece placement & related properties
        // - Normal move can be either quiet or capture
        if (move.is_capture()) {
            m_pstack.top().captured = m_board[to];
            // TODO: posInfo->gameStageValue = posInfo->prev->gameStageValue - Evaluation::PieceStageInfluence[posInfo->capturedPiece];

            m_zobrist.update(m_board[to], to);
            remove_piece(to);
        }
        else 
            m_pstack.top().game_stage = m_pstack.top_n(1).game_stage;   // No capture = no change in game stage

        m_zobrist.update(piece, from);
        m_zobrist.update(piece, to);
        move_piece(from, to);

        // Step 2 - update castling rights
        // - For more exaplanation, see CastleLoss table comments
        m_zobrist.update(m_pstack.top_n(1).castling_rights);
        m_pstack.top().castling_rights = m_pstack.top_n(1).castling_rights & ~CastleLoss[from] & ~CastleLoss[to];
        m_zobrist.update(m_pstack.top().castling_rights);

        // Step 3 - update enpassant square
        // - Every normal move resets enpassant square except double pawn pushes
        m_zobrist.update(m_pstack.top_n(1).enpassant_square);
        m_pstack.top().enpassant_square = move.is_double_pawn_push() && adjacent_rank_squares(to) & pieces(~m_moving_side, PAWN) ? to : NULL_SQUARE;
        m_zobrist.update(m_pstack.top().enpassant_square);

        // Step 4 - other updates
        m_pstack.top().halfmove_clock = move.is_capture() || type_of(piece) == PAWN ? 0 : m_pstack.top_n(1).halfmove_clock + 1;
        m_pstack.top().irr_distance = move.is_capture() || type_of(piece) == PAWN ? 0 : m_pstack.top_n(1).irr_distance + 1;
    }

    // Specialized move makers - promotions
    void Board::make_promotion(const Move& move)
    {
        // We know that moving piece is a pawn
        Square from = move.from();
        Square to = move.to();
        Piece promoted = make_piece(m_moving_side, move.promotion_type());

        // Step 1 - update piece placement & related properties
        // - Promotions, similarly to normal moves, can rither capture something or not
        // - We handle promotions with remove_piece + place_piece instead of move_piece to change piece type

        // Update piece value sum (promotion brings new piece to the board)
        m_pstack.top().game_stage = m_pstack.top_n(1).game_stage + 0;   // TODO: add promotion piece real value

        if (move.is_capture()) {
            m_pstack.top().captured = m_board[to];
            m_pstack.top().game_stage -= 0;                             // TODO: subtract captured piece real value

            m_zobrist.update(m_board[to], to);
            remove_piece(to);
        }
        
        m_zobrist.update(m_board[from], from);
        remove_piece(from);
        m_zobrist.update(promoted, to);
        place_piece(promoted, to);

        // Step 2 - update castling rights
        // - Promotion cannot affect castling rights unless it comes with a capture of enemy rook
        m_pstack.top().castling_rights = m_pstack.top_n(1).castling_rights;
        if (move.is_capture()) {
            m_zobrist.update(m_pstack.top_n(1).castling_rights);
            m_pstack.top().castling_rights &= ~CastleLoss[to];
            m_zobrist.update(m_pstack.top().castling_rights);
        }

        // Step 3 - update enpassant square
        // - Promotion always reset enpassant square
        m_zobrist.update(m_pstack.top_n(1).enpassant_square);
        m_pstack.top().enpassant_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        // Step 4 - other updates
        m_pstack.top().halfmove_clock = 0;
        m_pstack.top().irr_distance = 0;
    }

    // Specialized move makers - enpassant
    void Board::make_enpassant(const Move& move)
    {
        // We know that moving piece is a pawn
        Square from = move.from();
        Square to = move.to();
        Square capture_square = m_pstack.top_n(1).enpassant_square;

        // Step 1 - update piece placement & related properties
        // - Enpassant is by definition always a capture
        // - Unlike the normal captures, enpassant captures the pawn outside the target (to) square - on enpassant square

        m_pstack.top().captured = m_board[capture_square];
        m_pstack.top().game_stage = m_pstack.top_n(1).game_stage;

        m_zobrist.update(m_board[capture_square], capture_square);
        remove_piece(capture_square);
        m_zobrist.update(m_board[from], from);
        m_zobrist.update(m_board[from], to);
        move_piece(from, to);

        // Step 2 - update castling rights
        // - Enpassant cannot affect castling rights in any way
        m_pstack.top().castling_rights = m_pstack.top_n(1).castling_rights;

        // Step 3 - update enpassant square
        // - Enpassant always resets enpassant square
        m_zobrist.update(capture_square);   // capture_square is simultanously an enpassant square from previous ply
        m_pstack.top().enpassant_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        // Step 4 - other updates
        m_pstack.top().halfmove_clock = 0;
        m_pstack.top().irr_distance = 0;
    }

    // Specialized move makers - castle
    void Board::make_castle(const Move& move)
    {
        // Move contains starting and target square for king shift
        Square king_from = move.from();
        Square king_to = move.to();
        Square rook_from = make_square(rank_of(king_to), file_of(king_to) == ::FILE_G ? ::FILE_H : ::FILE_A);
        Square rook_to = make_square(rank_of(king_to), file_of(king_to) == ::FILE_G ? ::FILE_F : ::FILE_D);

        // Step 1 - update piece placement & related properties
        // - Castle consists of two independent moves: king move by 2 squares, and appropriate rook move
        m_pstack.top().game_stage = m_pstack.top_n(1).game_stage;

        m_zobrist.update(m_board[king_from], king_from);
        m_zobrist.update(m_board[king_from], king_to);
        move_piece(king_from, king_to);
        m_zobrist.update(m_board[rook_from], rook_from);
        m_zobrist.update(m_board[rook_from], rook_to);
        move_piece(rook_from, rook_to);

        // Step 2 - update castling rights
        // - Castling discards all the castling rights for castling side
        // - Equivalent to masking by castle loss for king move
        m_zobrist.update(m_pstack.top_n(1).castling_rights);
        m_pstack.top().castling_rights = m_pstack.top_n(1).castling_rights & ~CastleLoss[king_from];
        m_zobrist.update(m_pstack.top().castling_rights);

        // Step 3 - update enpassant square
        // - Castling always resets enpassant square
        m_zobrist.update(m_pstack.top_n(1).enpassant_square);
        m_pstack.top().enpassant_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        // Step 4 - other updates
        // - Castling is considered irreversible move, but it does not reset halfmove clock
        m_pstack.top().halfmove_clock = m_pstack.top_n(1).halfmove_clock + 1;
        m_pstack.top().irr_distance = 0;
    }

    // Unmaking moves
    // - NOTE: since zobrist hash is already stored on position stack, we can ommit any dynamic upates of zobrist hashing object
    void Board::undo_move()
    {
        // If positon stack is empty, then there are no more moves to be unmade
        if (m_pstack.empty())
            return;

        const Move& last_move = m_pstack.top().last_move;
        Square from = last_move.from();
        Square to = last_move.to();

        // Step 1 - revert piece placement changes
        // - Reverting piece move can be done by moving with reverse from-to squares
        // - Additionally, all captured pieces must be placed back onto the board
        switch (last_move.type()) {
            case Moves::NORMAL:
                move_piece(to, from);
                if (last_move.is_capture()) 
                    place_piece(m_pstack.top().captured, to);
                break;
            case Moves::PROMOTION:
                remove_piece(to);
                if (last_move.is_capture())
                    place_piece(m_pstack.top().captured, to);
                place_piece(make_piece(~m_moving_side, PAWN), from);
                break;
            case Moves::ENPASSANT:
                move_piece(to, from);
                place_piece(m_pstack.top().captured, m_pstack.top_n(1).enpassant_square);
                break;
            case Moves::CASTLE:
                move_piece(to, from);	// King
                move_piece(make_square(rank_of(to), file_of(to) == ::FILE_G ? ::FILE_F : ::FILE_D),
                           make_square(rank_of(to), file_of(to) == ::FILE_G ? ::FILE_H : ::FILE_A));	// Rook
                break;
            default:
                return;
        }

        // Step 2 - undo side to move and halfmove counter changes
        m_moving_side = ~m_moving_side;
        m_halfmoves--;

        // Step 3 - revert all the other changes by decrementing position stack
        m_pstack.pop();

        // Step 4 - restore zobrist hash
        m_zobrist.set(m_pstack.top().hash);
    }


    // -------------------------------------------------------------------
    // Board - position change - dynamic (move make & unmake) - null moves
    // -------------------------------------------------------------------

    // Specialized null move (passing move) maker
    void Board::make_null_move()
    {
        // Push position stack to make room for new ply data
        m_pstack.push();

        // Null move is a passing move with the following properties:
        // - Null move changes side to move
        // - Null move does not affect piece placement and castling rights
        // - Null move might reset enpassant square if it's not a NULL_SQUARE yet
        // - Null move requires update_checks() to be called, since this method is relative to current side to move

        // Change side to move
        m_moving_side = ~m_moving_side;
        m_zobrist.update(m_moving_side);

        // Copy position data and change only relevant properties
        m_pstack.top() = m_pstack.top_n(1);

        m_pstack.top().last_move = Moves::null;
        m_pstack.top().captured = NO_PIECE;

        m_zobrist.update(m_pstack.top_n(1).enpassant_square);
        m_pstack.top().enpassant_square = NULL_SQUARE;
        m_zobrist.update(NULL_SQUARE);

        m_pstack.top().hash = m_zobrist.hash();

        // Calculate checks for new side to move
        update_checks();

        // NOTE: since null move is not really a legal move, we do not increase halfmoves counter
    }

    // Specialized null move (passing move) unmaker
    void Board::undo_null_move()
    {
        // Since null move does not affect piece placement, reverting it is very simple
        m_moving_side = ~m_moving_side;
        m_pstack.pop();
        m_zobrist.set(m_pstack.top().hash);
    }


    // -----------------------------------------------------
    // Board - position analysis - square centric operations
    // -----------------------------------------------------

    Bitboard Board::attackers_to(Square sq, Bitboard occ) const
    {
        return Pieces::pawn_attacks(WHITE, sq) & pieces(BLACK, PAWN) | 
		       Pieces::pawn_attacks(BLACK, sq) & pieces(WHITE, PAWN) |
		       Pieces::piece_attacks_s<KNIGHT>(sq) & pieces(KNIGHT) |
		       Pieces::piece_attacks_s<BISHOP>(sq, occ) & pieces(BISHOP, QUEEN) |
		       Pieces::piece_attacks_s<ROOK>(sq, occ) & pieces(ROOK, QUEEN) |
		       Pieces::piece_attacks_s<KING>(sq) & pieces(KING);
    }


    // ---------------------------------------
    // Board - position analysis - repetitions
    // ---------------------------------------

    std::pair<uint16_t, uint16_t> Board::repetitions() const
    {
        // Repetition of a position requires at least 2 moves from both sides - one to go to a different position, and one to go back
        // For this reason we know that repetition cannot happen if last irreversible move came less then 2 moves (4 plies) ago
        if (irreversible_distance() < 4)
            return std::make_pair<uint16_t, uint16_t>(1, 0);

        Zobrist::Hash curr_hash = m_pstack.top().hash;

        uint16_t count = 1;                     // We are already counting the current position as first occurance
        uint16_t distance = 0;

        // Since repetition of current position can happen only at current side to move, we can limit search to only
        // plies with current side to move as a moving side
        // - Determines the maximal range of search inside position stack
        uint16_t loops = m_pstack.top().irr_distance / 2;     
        
        for (int i = 1; i <= loops; i++) {
            if (m_pstack.size() < 2 * i + 1)
                break;
            
            // We assume that positions are the same if their hashes are equal, altrough it's not always true
            // (due to limited range of hash values)
            if (curr_hash == m_pstack.top_n(2 * i).hash) {
                count++;
                distance = distance == 0 ? m_pstack.top().halfmove_clock - m_pstack.top_n(2 * i).halfmove_clock : distance;
            }
        }

        return std::make_pair(count, distance);
    }


    // -----------------------------------
    // Board - position analysis - threats
    // -----------------------------------

    Bitboard Board::threats(Color side)
    {
        // Piece maps
        Bitboard threat_map = 0;

        // Attack maps
        // - Defense map is static, while attack map is dynamically updated after considering threats for each piece type
        // - Attack map initially contains attacks on undefended squares and we simply add lower type attacks step by step
        Bitboard defense_map = attacks(side);
        Bitboard attack_map = attacks(~side) & ~defense_map | attacks(~side, PAWN);

        // Threats against knights & bishops
        threat_map |= pieces(side, KNIGHT, BISHOP) & attack_map;
        attack_map |= attacks(~side, KNIGHT) | attacks(~side, BISHOP);      // Update attack maps, since next piece type is rook

        // Threats against rooks
        threat_map |= pieces(side, ROOK) & attack_map;
        attack_map |= attacks(~side, ROOK);                                 // Update attack maps, since next piece type is queen

        // Threats against queens
        threat_map |= pieces(side, QUEEN) & attack_map;

        return threat_map;
    }


    // ---------------------------------------
    // Board - move analysis - legality checks
    // ---------------------------------------

    bool Board::is_pseudolegal(const Move& move) const
    {
        Square from = move.from(), to = move.to();
        Piece piece = m_board[from];

        // There has to be a piece on starting square of the move
        if (piece == NO_PIECE)
            return false;

        Color side = color_of(piece);
        Color enemy = ~side;
        Direction forward = side == WHITE ? NORTH : SOUTH;

        if (side !=m_moving_side)
            return false;

        // If target square is not empty, it cannot be occupied by friendly piece and move has to be capture
        if (m_board[to] != NO_PIECE && (!move.is_capture() || color_of(m_board[to]) == side))
            return false;

        // If target square is empty, then move cannot be a capture
        if (m_board[to] == NO_PIECE && move.is_capture() && !move.is_enpassant())
            return false;

        // Ensure that move is a check evasion if the side is in check
        if (in_check()) {
            // Piece moves
            if (type_of(piece) != KING) {
                // A piece move can never be a check evasion in case of double check
                if (!Bitboards::singly_populated(m_pstack.top().checkers))
                    return false;

                Square checkSquare = Bitboards::lsb(m_pstack.top().checkers);
                if (to != checkSquare &&
                    (!move.is_enpassant() || to != enpassant_square() + forward) &&
                    !aligned_in_order(m_kings[side], to, checkSquare))
                    return false;
            }
            // King moves
            else if (attackers_to(to, enemy, pieces() ^ from))
                return false;
        }

        // Special case - castle
        if (move.is_castle()) {
            if (!can_castle(side, move.castle_type()) || !castle_path_clear(side, move.castle_type()) || in_check())
                return false;
        }
        // Special case - pawn moves
        else if (type_of(piece) == PAWN) {
            Bitboard secondRank = side == WHITE ? RANK_2 : RANK_7;

            if (!(move.is_enpassant() && enpassant_square() != NULL_SQUARE &&
                  adjacent_rank_squares(enpassant_square()) & from && to == enpassant_square() + forward) &&               // Enpassant														// Enpassant
                !(move.is_capture() && !move.is_enpassant() && Pieces::pawn_attacks(side, from) & to) &&                   // Capture
                !(to == from + forward && m_board[to] == NO_PIECE) &&                                                      // 1-push
                !(to == from + forward + forward && !(Paths[from][to] & (pieces() ^ from)) && secondRank & from))          // 2-push
                return false;
        }
        // Common moves - special flags must be 0
        else if (move.flags() & 0xb)
            return false;
        // Common moves - moving correctness
        else if (!(Pieces::piece_attacks_d(type_of(piece), from, pieces()) & to))
            return false;

        return true;
    }

    bool Board::is_legal_p(const Move& move) const
    {
        Square from = move.from(), to = move.to();
        Piece piece = m_board[from];

        Color side = color_of(piece);
        Color enemy = ~side;

        // Special case - enpassant
        // It requires a check whether a move would create a discovered attack against our king
        if (move.is_enpassant()) {
            Bitboard occ = (pieces() ^ enpassant_square() ^ from) | to;
            return !(Pieces::piece_attacks_s<BISHOP>(m_kings[side], occ) & pieces(enemy, BISHOP, QUEEN)) &&
                   !(Pieces::piece_attacks_s<ROOK>(m_kings[side], occ) & pieces(enemy, ROOK, QUEEN));
        }

        // Special case - castle
        if (move.is_castle()) {
            Direction dir = to > from ? EAST : WEST;

            // King can't castle via attacked squares
            return !attackers_to(from + dir, enemy, pieces()) &&
                   !attackers_to(to, enemy, pieces());
        }

        // King can't step into a square attacked by enemy piece
        if (type_of(piece) == KING)
            return !attackers_to(to, enemy, pieces() ^ from);

        // Finally, check for pins
        return !(pinned(side) & from) || aligned_in_order(m_kings[side], from, to) ||
                                         aligned_in_order(m_kings[side], to, from);
    }


    // --------------------------------------------------------
    // Board - move analysis - SEE (static exchange evaluation)
    // --------------------------------------------------------

    // Helper function - finding the least valuable attacker
    // - Saves the least valuable attacker piece type to type parameter
    // - We assume that piece types are already ordered by increasing value (which in fact, is true for PieceType enum)
    Square lvp(const Board& board, Color side, Bitboard area, PieceType &type)
    {
        for (type = PAWN; type <= KING; type = PieceType(type + 1))
        {
            // area is just a set of squares from which we try to find least valuable attackers
            Bitboard attackers = area & board.pieces(side, type);

            // First found attacker must be of the lowest value
            if (attackers)
                return Bitboards::lsb(attackers);
        }

        return NULL_SQUARE;
    }

    // Main SEE procedure
    int32_t Board::see(Square from, Square to, PieceType promote_to) const
    {
        // First, let's cover a null move case
        if (from == to)
            return 0;
        
        // Gain is a helper table used to calculate material balance change after best exchanges in each ply
        // - Since there are no more than 32 pieces on the board, there can be at most 32 exchanges (we include kings for simplicity)
        int32_t gain[33];

        // Depth is an index to gain table
        int depth = int(m_moving_side);

        PieceType attacked = type_of(on(to));
        PieceType attacker = type_of(on(from));

        // - attackdef is a map covering all pieces attacking given square (from both sides)
        // - possible_xray is a set of all sliding pieces and pawns
        Bitboard occ = pieces();
        Bitboard attackdef = attackers_to(to, occ);
        Bitboard possible_xray = pieces() ^ pieces(KNIGHT) ^ pieces(KING);

        // Special case - quiet pawn move
        // - Pawns are special in the way that pushing them can never be capture
        // - Updating attackdef in this case is crucial for method to not forget to include pawn value in exchanges
        if (attacker == PAWN && file_of(from) == file_of(to))
            attackdef |= from;

        gain[depth] = Evaluation::PieceValues[attacked] + Evaluation::PieceValues[promote_to]
                                                        - Evaluation::PieceValues[PAWN];

        do {
            depth++;

            gain[depth] = Evaluation::PieceValues[attacker] - gain[depth - 1];

            // No point to go any further if last best capture resulted in loss of material
            if (std::max(-gain[depth - 1], gain[depth]) < 0)
                break;
            
            attackdef = attackdef ^ from;
            if (possible_xray & from) {
                Bitboard xray_attackers = (Pieces::xray_attacks<BISHOP>(to, occ, square_to_bb(from)) & pieces(BISHOP, QUEEN)) |
										   (Pieces::xray_attacks<ROOK>(to, occ, square_to_bb(from)) & pieces(ROOK, QUEEN));
				attackdef |= xray_attackers;
            }

            // Remove piece that made a capture from occupancy set
            occ = occ ^ from;

            // Find next best (least valuable) attacker
            from = lvp(*this, Color(depth & 0x1), attackdef, attacker);
        } while (from != NULL_SQUARE);

        while (--depth)
			gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);

		return gain[m_moving_side];
    }


    // ----------------------------------------
    // Board - move analysis - other properties
    // ----------------------------------------

    bool Board::is_check(const Move& move) const
    {
        Square from = move.from();
        Square to = move.to();

        return m_pstack.top().check_areas[type_of(m_board[from])] & to ||
               (m_pstack.top().discoveries[m_moving_side] & from &&
                !aligned_in_order(m_kings[~m_moving_side], to, from) &&
                !aligned_in_order(m_kings[~m_moving_side], from, to));
    }


    // ---------------------------------------------------
    // Board - helper functions - piece placement handlers
    // ---------------------------------------------------

    void Board::place_piece(Piece piece, Square sq)
    {
        // Update piece tables by adding a piece
        m_board[sq] = piece;
	    m_pieces_c[color_of(piece)] |= sq;
	    m_pieces_t[type_of(piece)] |= sq;
	    m_pieces_t[ALL_PIECES] |= sq;

        // Update king position in case of placing a king
	    if (type_of(piece) == KING) 
            m_kings[color_of(piece)] = sq;
    }

    void Board::remove_piece(Square sq)
    {
        // Get the piece standing on sq
        Piece piece = on(sq);

        // Update piece tables by removing a piece from square sq
        if (piece != NO_PIECE) {
            m_board[sq] = NO_PIECE;
	        m_pieces_c[color_of(piece)] ^= sq;
	        m_pieces_t[type_of(piece)] ^= sq;
	        m_pieces_t[ALL_PIECES] ^= sq;
        }

        // WARNING: We assume that king can never be captured or removed from the board,
        //          so we ignore updating the king position.
    }

    void Board::move_piece(Square from, Square to)
    {
        // Get the piece standing on starting square (from)
        Piece piece = on(from);

        // Update piece tables by moving a piece
        if (piece != NO_PIECE) {
            m_board[from] = NO_PIECE;
            m_board[to] = piece;

            // Calculate move shift map
            Bitboard movemap = square_to_bb(from) | square_to_bb(to);

            // By applying map with both from and to bits, we can perform the update in just 1 operation instead of 2
            m_pieces_c[color_of(piece)] ^= movemap;
	        m_pieces_t[type_of(piece)] ^= movemap;
	        m_pieces_t[ALL_PIECES] ^= movemap;

            // Update king position in case of moving the king
            if (type_of(piece) == KING) 
                m_kings[color_of(piece)] = to;
        }
    }


    // -----------------------------------------------
    // Board - helper functions - checks & pins update
    // -----------------------------------------------

    void Board::update_checks()
    {
        // To detect pieces that check the king, we can simply detect all attackers to king's position
        m_pstack.top().checkers = attackers_to(m_kings[m_moving_side], ~m_moving_side);

        // Detecting check areas works similarly, but instead of aggregative attackers_to we use 
        // specialized piece-attack functions.
        m_pstack.top().check_areas[PAWN] = Pieces::pawn_attacks(~m_moving_side, m_kings[~m_moving_side]);
	    m_pstack.top().check_areas[KNIGHT] = Pieces::piece_attacks_s<KNIGHT>(m_kings[~m_moving_side], pieces());
	    m_pstack.top().check_areas[BISHOP] = Pieces::piece_attacks_s<BISHOP>(m_kings[~m_moving_side], pieces());
	    m_pstack.top().check_areas[ROOK] = Pieces::piece_attacks_s<ROOK>(m_kings[~m_moving_side], pieces());
	    m_pstack.top().check_areas[QUEEN] = m_pstack.top().check_areas[BISHOP] | 
                                            m_pstack.top().check_areas[ROOK];
    }

    void Board::update_pins(Color side)
    {
        // For readability
        Color enemy = ~side;

        // Reset relevant tables first
        m_pstack.top().pinned[side] = 0;
        m_pstack.top().pinners[enemy] = 0;
        m_pstack.top().discoveries[enemy] = 0;

        // We can express pin as a some sort of an x-ray attack, but with piece of opposite color as a blocker
        // - NOTE: Here we also take discovered attack into consideretion (x-ray with same side piece as a blocker)
        Bitboard xray_attackers = (Pieces::xray_attacks<ROOK>(m_kings[side], pieces(), pieces()) & pieces(enemy, ROOK, QUEEN)) |
							      (Pieces::xray_attacks<BISHOP>(m_kings[side], pieces(), pieces()) & pieces(enemy, BISHOP, QUEEN));
        
        // Iterate over x-ray attackers map to extract each pin square
        while (xray_attackers) {
            Square sq = Bitboards::pop_lsb(xray_attackers);
            Bitboard discovery = (Paths[m_kings[side]][sq] & pieces(enemy)) ^ sq;

            // Potential discovery
            if (discovery)
                m_pstack.top().discoveries[enemy] |= discovery;
            // If it's not a potential discovery, then it's a pin
            else {
                m_pstack.top().pinned[side] |= Paths[m_kings[side]][sq] & pieces(side);
                m_pstack.top().pinners[enemy] |= sq;
            }
        }

        // Discard king square from pin set, since king cannot be pinned
        // - The above algorithm can incorrectly classify king as pinned in case of a check
        m_pstack.top().pinned[side] &= ~m_kings[side];
    }


    // ---------------------------------------------
    // Board - helper functions - attack maps update
    // ---------------------------------------------

    void Board::update_attacks()
    {
        // Update for both sides
        for (unsigned side = WHITE; side <= BLACK; side++) {
            // Pawns, knights and kings can be covered separately using aggregative attack calculations
            m_pstack.top().attacks[side][PAWN] = side == WHITE ? Pieces::pawn_attacks<WHITE>(pieces(WHITE, PAWN)) :
                                                                 Pieces::pawn_attacks<BLACK>(pieces(BLACK, PAWN));
            m_pstack.top().attacks[side][KNIGHT] = Pieces::knight_attacks(pieces(Color(side), KNIGHT));
            m_pstack.top().attacks[side][KING] = Pieces::piece_attacks_s<KING>(king_position(Color(side)));

            // For sliding piece attacks, we need to cover each piece indyvidualy
            Bitboard bishops = pieces(Color(side), BISHOP);
            while (bishops)
                m_pstack.top().attacks[side][BISHOP] |= Pieces::piece_attacks_s<BISHOP>(Bitboards::pop_lsb(bishops), pieces());
            
            Bitboard rooks = pieces(Color(side), ROOK);
            while (rooks)
                m_pstack.top().attacks[side][ROOK] |= Pieces::piece_attacks_s<ROOK>(Bitboards::pop_lsb(rooks), pieces());

            Bitboard queens = pieces(Color(side), QUEEN);
            while (queens)
                m_pstack.top().attacks[side][QUEEN] |= Pieces::piece_attacks_s<QUEEN>(Bitboards::pop_lsb(queens), pieces());
            
            // Finally, calculate all piece attack map
            m_pstack.top().attacks[side][ALL_PIECES] = m_pstack.top().attacks[side][PAWN] |
                                                       m_pstack.top().attacks[side][KNIGHT] |
                                                       m_pstack.top().attacks[side][BISHOP] |
                                                       m_pstack.top().attacks[side][ROOK] |
                                                       m_pstack.top().attacks[side][QUEEN] |
                                                       m_pstack.top().attacks[side][KING];
        }

        m_pstack.top().attacks_ready = true;
    }


    // --------------------
    // Board - comparisions
    // --------------------

    bool Board::operator==(const Board& other) const
    {
        return m_moving_side == other.m_moving_side &&
               std::equal(m_board, m_board + SQUARE_RANGE, other.m_board) &&
               std::equal(m_pieces_c, m_pieces_c + COLOR_RANGE, other.m_pieces_c) &&
               std::equal(m_pieces_t, m_pieces_t + PIECE_TYPE_RANGE, other.m_pieces_t) &&
               std::equal(m_kings, m_kings + COLOR_RANGE, other.m_kings) &&
               castling_rights() == other.castling_rights() &&
               enpassant_square() == other.enpassant_square() &&
               checkers() == other.checkers() &&
               std::equal(m_pstack.top().check_areas, m_pstack.top().check_areas + PIECE_TYPE_RANGE, other.m_pstack.top().check_areas) &&
               std::equal(m_pstack.top().discoveries, m_pstack.top().discoveries + COLOR_RANGE, other.m_pstack.top().discoveries) &&
               std::equal(m_pstack.top().pinned, m_pstack.top().pinned + COLOR_RANGE, other.m_pstack.top().pinned) &&
               std::equal(m_pstack.top().pinners, m_pstack.top().pinners + COLOR_RANGE, other.m_pstack.top().pinners) &&
               game_stage() == other.game_stage();
    }


    // --------------------------
    // Board - FEN representation
    // --------------------------

    std::string Board::fen() const
    {
        // Let's start with an empty string object and fill it in consecutive steps
        std::ostringstream fen;

        // Iterate over all squares in top -> down and left -> right order (as denoted in FEN)
        for (int r = ::RANK_8; r >= 0; r--) {
            int gap = 0;

            for (int f = ::FILE_A; f <= ::FILE_H; f++) {
                Piece piece = m_board[make_square(Rank(r), File(f))];

                if (piece != NO_PIECE) {
                    if (gap > 0)
                        fen << gap;
                    gap = 0;

                    fen << piece;
                }
                else
                    gap++;
            }

            if (gap > 0)
                fen << gap;
            if (r > 0)
                fen << "/";
        }

        // Side to move
        fen << " " << (m_moving_side == WHITE ? "w" : "b") << " ";

        // Castling rights
        if (can_castle(WHITE, KINGSIDE_CASTLE))
            fen << "K";
        if (can_castle(WHITE, QUEENSIDE_CASTLE))
            fen << "Q";
        if (can_castle(BLACK, KINGSIDE_CASTLE))
            fen << "k";
        if (can_castle(BLACK, QUEENSIDE_CASTLE))
            fen << "q";
        if (castling_rights() == NO_CASTLING)
            fen << "-";
        
        // Enpassant
        fen << " ";
        if (enpassant_square() != NULL_SQUARE && rank_of(enpassant_square()) == ::RANK_4 &&
            adjacent_rank_squares(enpassant_square()) & pieces(~m_moving_side, PAWN))
            fen << make_square(::RANK_3, file_of(enpassant_square()));
        else if (enpassant_square() != NULL_SQUARE && 
                 adjacent_rank_squares(enpassant_square()) & pieces(~m_moving_side, PAWN))
            fen << make_square(::RANK_6, file_of(enpassant_square()));
        else
            fen << "-";

        // Halfmove clock
        fen << " " << halfmoves_c();

        // Move count
        fen << " " << moves_p();

        return fen.str();
    }

}