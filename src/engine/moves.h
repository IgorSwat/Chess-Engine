#pragma once

#include "types.h"
#include "../utilities/sarray.h"
#include <optional>


/*
    ---------- Moves ----------

    This file contains type definitions for basic move representations
    - Basic enums and flags serve as ingredients for building more complex move representations
    - Move class and derived classes interact with engine components
*/

namespace Moves {

    // ------------------------------
    // Basic definitions - move types
    // ------------------------------

    // We can distinguish 4 main types of moves: standard moves, and 3 special categories (enpassant, castle and promotion)
    // - The categories are completely separate
    // - Standard moves and promotions can be further divided into captures and non-captures, while enpassant is always a capture
    enum MoveType : uint32_t {
        NORMAL = 1,
        PROMOTION,
        ENPASSANT,
        CASTLE,

        // A special category used only in the context of null move instance
        NULL_MOVE = 0
    };


    // ------------------------------
    // Basic definitions - move flags
    // ------------------------------

    // A unique identifier of a move. Consists of (from most important to least important bits):
    // - 4 bits for move flags (promotion bit, capture bit, and two special bits indicating special types of moves)
    // - 6 bits for target (to) square
    // - 6 bits for start (from) square
    using Mask = uint16_t;

    // Flags are part of full move mask - more specyfically, 4 most important bits
    // - To map flags into correct place in move mask, one should shift flags left by 12 bits
    // - NOTE: all flags must be possible to save using 4 bits (not exceed 0xf)
    using Flags = Mask;

    // Predefined flags - elementary
    constexpr Flags SPECIAL1_FLAG = 0x1;
    constexpr Flags SPECIAL2_FLAG = 0x2;
    constexpr Flags CAPTURE_FLAG = 0x4;
    constexpr Flags PROMOTION_FLAG = 0x8;

    // Predefined flags - complex
    // - Built by combining elementary flags and giving them context
    constexpr Flags QUIET_MOVE_FLAG         = 0x0;
    constexpr Flags NON_QUIET_MOVE_FLAG     = CAPTURE_FLAG | PROMOTION_FLAG;
    constexpr Flags DOUBLE_PAWN_PUSH_FLAG   = SPECIAL1_FLAG;
    constexpr Flags ENPASSANT_FLAG          = CAPTURE_FLAG | SPECIAL1_FLAG;
    constexpr Flags KINGSIDE_CASTLE_FLAG    = SPECIAL2_FLAG;
    constexpr Flags QUEENSIDE_CASTLE_FLAG   = SPECIAL2_FLAG | SPECIAL1_FLAG;
    constexpr Flags KNIGHT_PROMOTION_FLAG   = PROMOTION_FLAG;
    constexpr Flags BISHOP_PROMOTION_FLAG   = PROMOTION_FLAG | SPECIAL1_FLAG;
    constexpr Flags ROOK_PROMOTION_FLAG     = PROMOTION_FLAG | SPECIAL2_FLAG;
    constexpr Flags QUEEN_PROMOTION_FLAG    = PROMOTION_FLAG | SPECIAL2_FLAG | SPECIAL1_FLAG;

    // Specify move category based on given move flags
    constexpr inline MoveType type_of(Flags flags)
    {
        return flags >= PROMOTION_FLAG ? PROMOTION :
               flags == ENPASSANT_FLAG ? ENPASSANT :
               flags == KINGSIDE_CASTLE_FLAG || flags == QUEENSIDE_CASTLE_FLAG ? CASTLE : NORMAL;
    }


    // ------------------
    // Moves - plain move
    // ------------------

    // The primary idea behind the following class is to provide an useful abstraction for chess move
    // - It only covers the logic behind chess move, does not contain any additional info about move
    class Move
    {
    public:
        // Constructors - for both static and dynamic initialization
        constexpr Move() = default;
        Move(Square from, Square to, Flags flags) :
            m_move((flags & 0xf) << 12 | Mask(to) << 6 | Mask(from)) {}

        // Getters - move squares
        Square from() const { return Square(m_move & 0x3f); }
	    Square to() const { return Square((m_move >> 6) & 0x3f); }

        // Getters - mask & flags
        Mask raw() const { return m_move; }                 // Returns whole mask (a complete move identifier)
        Mask butterfly() const { return m_move & 0x0fff; }  // Returns a butterfly index (indentyfing move by from and to squares)
        Flags flags() const { return m_move >> 12; }        // Returns just a move flags (specyfing move type)

        // Getters - move type & properties
        MoveType type() const { return type_of(flags()); }
        PieceType promotion_type() const { return is_promotion() ? PieceType((flags() & 0x3) + 2) : NULL_PIECE_TYPE; }
        Castle castle_type() const { return is_castle() ? Castle((flags() & 0x3) - 1) : NULL_CASTLE; }
        bool is_capture() const { return flags() & CAPTURE_FLAG; }
        bool is_promotion() const { return flags() & PROMOTION_FLAG; }
        bool is_quiet() const { return !(flags() & NON_QUIET_MOVE_FLAG); }
        bool is_double_pawn_push() const { return flags() == DOUBLE_PAWN_PUSH_FLAG; }
        bool is_enpassant() const { return flags() == ENPASSANT_FLAG; }
        bool is_castle() const { return flags() == KINGSIDE_CASTLE_FLAG || flags() == QUEENSIDE_CASTLE_FLAG; }

        // Logical operators - comparisions
        friend bool operator==(const Move& m1, const Move& m2) { return m1.m_move == m2.m_move; }
	    friend bool operator!=(const Move& m1, const Move& m2) { return m1.m_move != m2.m_move; }

        // Printing
        friend std::ostream& operator<<(std::ostream& os, const Move& move);

    protected:
        Mask m_move = 0;
    };


    // ---------------------
    // Moves - enhanced move
    // ---------------------

    // Enhancement definition
    // - Enhancement is some additional information about move, embedded together with Move object itself
    // - It could be some evaluation performed on a move (like SEE), or just a custom number which serves as a sorting index
    enum class Enhancement : uint8_t {
        PURE_SEE = 1,
        PURE_SEARCH_SCORE,
        CUSTOM_SORTING,

        NONE = 0
    };

    // Enhanced move
    // - Decorator pattern: move + additional info about the move
    // - Enables sorting move lists with this kind of moves in place
    class EnhancedMove : public Move
    {
    public:
        // Allow to construct enchancement move object from plain move object
        EnhancedMove() = default;
        EnhancedMove(Square from, Square to, Flags flags) : Move(from, to, flags) {}
	    EnhancedMove(const Move& move) : Move(move) {}
        EnhancedMove& operator=(const Move& other) { m_move = other.raw(); return *this; }

        // A convenient setter which makes sure that both enhancement type and enhancement key are set together
	    void enhance(Enhancement enhancement, int32_t value) { m_enhancement = enhancement; m_key = value; }

        // Standard getters
        int32_t key() const { return m_key; }
        Enhancement enhancement() const { return m_enhancement; }

        // Specialized getters
        // - Return key value only if enhancement matches correctly to the one provided as argument
        std::optional<int32_t> see() const { return m_enhancement == Enhancement::PURE_SEE ? std::make_optional(m_key) : std::nullopt; }
        std::optional<int32_t> score() const { return m_enhancement == Enhancement::PURE_SEARCH_SCORE ?
                                                                       std::make_optional(m_key) : std::nullopt; }

    private:
        Enhancement m_enhancement = Enhancement::NONE;      // Enchancement type
        int32_t m_key = 0;                                  // Enchancement value
    };

    // Useful type alias for shorten name
    using EMove = EnhancedMove;


    // -------------------------------------
    // Moves - other definitions - null move
    // -------------------------------------

    // Since we know that mask 0 indicates an invalid move, we can use it to represent null move
    // - In exact terms, we represent null move as a quiet move from A1 to A1
    inline constexpr Move null;


    // -------------------------------------
    // Moves - other definitions - move list
    // -------------------------------------

    // Move list is basically a stable array of arbitrary size than contains given types of moves (could be plain Move objects, or more advanced ones)
    // - By default 256 is a maximal size (the biggest known amount of legal moves is any chess position is 218, but we round to the power of 2)
    template <typename MoveT = Move, unsigned size = 256>
    using List = StableArray<MoveT, size>;

}

// Share commong usages from Moves namespace
using Moves::Move;
using Moves::EMove;