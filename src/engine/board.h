#pragma once

#include "boardspace.h"
#include "boardlogic.h"
#include "moves.h"
#include "pieces.h"
#include "zobrist.h"
#include "../utilities/sstack.h"
#include <stack>
#include <vector>


/*
    ---------- Board ----------

    Defines standard board representation in form of Board class
    - Board class remembers all the necessary aspects of a chess position
    - Board class contains additional logic for analyzing the position and potential moves
*/

namespace Chessboard {

    // ---------------------------
    // Basic constants and defines
    // ---------------------------

    const std::string STARTING_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


    // --------------------
    // Board representation
    // --------------------

    class Board
    {
    public:
        Board() { load_position(); }

        // Position change - static - position loading
        void clear();                                               // Make empty board
        void load_position() { load_position(STARTING_POSITION); }  // Loads starting position
        void load_position(const std::string& fen);                 // Loads position from given FEN notation
        void load_position(const Board& other);                     // Loads position from different Board object

        // Position change - dynamic - move make & unmake
        // - Most of data is placed on the stack to provide quick, reversible position change operation
        void make_move(const Move& move);
        void undo_move();                               // Undo only the last move, if any move was made
        void make_null_move();                          // Specialized null move (passing move) maker
        void undo_null_move();                          // Specialized null move (passing move) unmaker

        // Position analysis - piece-centric operations
        Bitboard pieces(Color side) const { return m_pieces_c[side]; }
        Bitboard pieces(PieceType ptype = ALL_PIECES) const { return m_pieces_t[ptype]; }
        template <typename... PieceTypes>
        Bitboard pieces(PieceType ptype, PieceTypes... types) const { return pieces(ptype) | pieces(types...); }
        template <typename... PieceTypes>
        Bitboard pieces(Color side, PieceTypes... types) const { return pieces(side) & pieces(types...); }
        Square king_position(Color side) const { return m_kings[side]; }

        // Position analysis - square-centric operations
        Piece on(Square sq) const { return m_board[sq]; }
        bool occupied(Square sq) const { return m_board[sq] != NO_PIECE; }
        Bitboard attackers_to(Square sq) const { return attackers_to(sq, pieces()); }   // Attackers from both sides
        Bitboard attackers_to(Square sq, Bitboard occ) const;
        Bitboard attackers_to(Square sq, Color side) const { return attackers_to(sq, side, pieces()); }   // Attackers from given side
        Bitboard attackers_to(Square sq, Color side, Bitboard occ) const { return attackers_to(sq, occ) & pieces(side); }

        // Position analysis - checks & pins
        // - NOTE: all of those methods assumes that position is legal and only side to move can be in check or give a check
        bool in_check() const { return m_pstack.top().checkers; }
        Bitboard checkers() const { return m_pstack.top().checkers; }
        Bitboard possible_checks(PieceType ptype) const { return m_pstack.top().check_areas[ptype]; }
        Bitboard pinned(Color side) const { return m_pstack.top().pinned[side]; }
        Bitboard pinners(Color side) const { return m_pstack.top().pinners[side]; }

        // Position analysis - special properties
        // - can_castle() checks only whether given side has appropriate castling rights (does not check legality of castling)
        // - castle_path_clear() checks only if a king path for given castle is clear of pieces
        CastlingRights castling_rights() const { return m_pstack.top().castling_rights; }
        bool can_castle(Color side, Castle castle) const { return  m_pstack.top().castling_rights & make_castle_right(side, castle); }
        bool castle_path_clear(Color side, Castle castle) const { return !(castle_path(side, castle) & pieces()); }
        Square enpassant_square() const { return m_pstack.top().enpassant_square; }

        // Position analysis - move counting
        // - halfmoves_p indicates plain counter of halfmoves - without reseting after captures and pawn moves
        // - halfmoves_c indicates clock counter of halfmoves - with reset after every capture or pawn move
        // - moves_p  is basically the same as halfmoves_p, but here we count move as a pair of one move from each side
        uint32_t halfmoves_p() const { return m_halfmoves; }
        uint32_t halfmoves_c() const { return m_pstack.top().halfmove_clock; }
        uint32_t moves_p() const { return (m_halfmoves + 2 - m_moving_side) / 2; }
        uint32_t irreversible_distance() const { return m_pstack.top().irr_distance; }
        uint32_t repetitions() const;       // Counts how many times current position has repeated

        // Position analysis - other
        Color side_to_move() const { return m_moving_side; }
        uint32_t game_stage() const { return m_pstack.top().game_stage; }
        Zobrist::Hash hash() const { return m_zobrist.hash(); }

        // Move analysis - legaity checks
        // - Pseudo legal move is a move that could be legal if we would ignore pins and potential attacks on the king.
        //   In other words, pseudo legal move is geometrically correct in given position
        // - Legal move is a pseudo legal move with proved correctness in the context of pins and attacks on the king
        bool is_pseudolegal(const Move& move) const;    // Test for pseudo legality
        bool is_legal_p(const Move& move) const;        // Test for legality, but assuming move is already pseudo legal (_p)
        bool is_legal_f(const Move& move) const { return is_pseudolegal(move) && is_legal_p(move); }       // Full test (_f)

        // Move analysis - SEE (Static Exchange Evaluation)
        // - SEE allows to evaluate a serie of captures initiated by given move statically - without search
        // - Always assumes that both sides will make optimal trades that maximizes their material gain
        int32_t see(Square from, Square to, PieceType promote_to = PAWN) const;
        int32_t see(const Move& move) const { return see(move.from(), move.to(), move.is_promotion() ? move.promotion_type() : PAWN); }

        // Move analysis - other properties
        bool is_check(const Move& move) const;

        // Comparisions
        // - Those comparisions compare all the logical aspects of two boards down to the top entry in position stack
        // - This means we do not compare the history of two boards, but just their current logical state
        // - For this reasons, we ommit checking halfmove counters and zobrist
        // - Mostly for testing / debugging purposes
        bool operator==(const Board& other) const;
        bool operator!=(const Board& other) const { return !(*this == other); }

        // FEN representation
        // - Reverse operation to load_position(fen)
        // - Can be used in various tests, as well as replacement for load_position(board)
        std::string fen() const;

        // Printing
        // - Uses FEN representation
        friend std::ostream& operator<<(std::ostream& os, const Board& board) { os << board.fen(); return os; }
        
    private:
        // Helper functions - move makers
        void make_normal(const Move& move);
        void make_promotion(const Move& move);
        void make_enpassant(const Move& move);
        void make_castle(const Move& move);

        // Helper functions - piece placement handlers
        void place_piece(Piece piece, Square sq);
        void remove_piece(Square sq);
        void move_piece(Square from, Square to);

        // Helper functions - checks & pins update
        // - NOTE: By pins for given side we mean pins that affect given side, not pins caused by the side
        void update_checks();                     // Updates all check informations for side to move
        void update_pins(Color side);             // Updates all pin related information (pins, pinners, discoveries) for given side

        // Common data
        // - Single instances inside Board class
        // - All the below variables should be dynamically updated each time a move is made or unmade
        Piece m_board[SQUARE_RANGE];
        Bitboard m_pieces_t[PIECE_TYPE_RANGE];    // Aggregative piece map indexed by piece type (_t) (pieces of both colors)
        Bitboard m_pieces_c[COLOR_RANGE];         // Aggregative piece map indexed by piece color (_c) (pieces of all types)
        Square m_kings[COLOR_RANGE];              // Additional lookup table for king position

        Color m_moving_side;

        uint32_t m_halfmoves = 0;   // Since undo operation on this is simple decrementation, we can store it as common data

        // Individual data - structure definition
        // - Aggregates all the data that needs to be stored individually for each ply
        struct Position
        {
            // Constructors
            Position() = default;
            Position(const Move& move) : last_move(move) {}

            // Assignments
            // - NOTE: since every field is either a single variable or static array, we can leave the implementation
            //         for compiler
            Position& operator=(const Position& other) = default;

            // Each position (excluding the one at ply 0) is reached from another one by making a move
            // - Storing the move made helps allows to obtain important information about how to undo that move
            // - To allow unmake of a move, we also must store the piece captured with the move (if any was captured)
            Move last_move = Moves::null;
            Piece captured = NO_PIECE;

            // Position data - checks & pins
            Bitboard checkers = 0ULL;							// Map of pieces that currently give a check (against side to move)
	        Bitboard check_areas[PIECE_TYPE_RANGE] = { 0ULL };	// Map of possible checks for current side to move pieces
	        Bitboard discoveries[COLOR_RANGE] = { 0ULL };		// Map of squares occupied by pieces which could cause a discovered check (for both sides)
	        Bitboard pinned[COLOR_RANGE] = { 0ULL };			// Map of pinned pieces (for both sides)
	        Bitboard pinners[COLOR_RANGE] = { 0ULL };			// Map of pieces that pin at least one of enemy's pieces ( for both sides)

            // Position data - special aspects
            CastlingRights castling_rights = NO_CASTLING;
            Square enpassant_square = NULL_SQUARE;

            // Position data - miscellaneous
            uint32_t halfmove_clock = 0;                   // Halfmove clock (resets after every irreversible move)
            uint32_t irr_distance = 0;                     // Irreversible distance - distance (in plies) from last irreversible move
            uint32_t game_stage = 0;

            // Position data - hash
            // - Storing hash allows us to quickly reset Zobrist object without performing all the operations
            Zobrist::Hash hash = 0;
        };

        // Individual data - unique for each ply
        // - Stack data structure, each element represents a single ply data
        StableStack<Position> m_pstack;

        // Hashing mechanism
        Zobrist::Zobrist m_zobrist;
    };

}

// Share commong usages from Chessboard namespace
using Chessboard::Board;