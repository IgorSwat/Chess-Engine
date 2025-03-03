#pragma once

#include "types.h"

// Forestall declarations
namespace Chessboard { class Board; }


/*
    ---------- Zobrist ----------

    Contains implementation of hashing mechanism called "Zobrist hash"
    - Zobrist hash is based on unique, 64-bit integers connected with every dynamic aspect of chess position,
      like piece placement, side to move, or castling rights
    - Zobrist has is dynamically updated each time something changes on the board
    - Dynamic update can be performed using logical XOR operation, which means it is reversible
    - The starting values of Zobrist numbers are randomly generated
*/

namespace Zobrist {

    // -------------
	// Initilization
	// -------------

    void initialize_zobrist_numbers();


    // ---------------------
	// Zobrist - definitions
	// ---------------------

    // Each Zobrist number is a single 64-bit hash value
    using Hash = uint64_t;

    // Each number is stored in precalculated table. Each element corresponds to given aspect of the position:
    // - 768 elements correspond to every possible placement of every piece (12 * 64)
    // - 16 elements correspond to any combination of castling rights
    // - 65 elements correspond to any possible enpassant square (including no enpassant available)
    // - 1 element corresponds to distinguish white to move vs black to move positions
    extern Hash ZobristNumbers[850];


    // ------------------------
	// Zobrist - main mechanism
	// ------------------------

    // The following class contains cumulative hash value and provides logic for both static and dynamic generation of hash
    class Zobrist
    {
    public:
        Zobrist() = default;

        // Static hash update
        void set(Hash hash) { m_hash = hash; }
        void generate(const Chessboard::Board& board);   // Generate hash from scratch for given position

        // Dynamic hash update
        void update(Piece piece, Square sq) { m_hash ^= ZobristNumbers[color_of(piece) * 384 + type_of(piece) * 64 + sq]; }  
        void update(CastlingRights rights)  { m_hash ^= ZobristNumbers[768 + rights]; }
        void update(Square epsquare)        { m_hash ^= ZobristNumbers[784 + epsquare]; }       
        void update(Color side2move)        { m_hash ^= ZobristNumbers[849]; }

        // Getters
        Hash hash() const { return m_hash; }

    private:
        // Combined hash
        Hash m_hash = 0;
    };

}