#include "zobrist.h"
#include "board.h"
#include "randomgen.h"
#include <algorithm>
#include <unordered_set>


namespace Zobrist {

    // ---------------------
	// Zobrist numbers table
	// ---------------------

    Hash ZobristNumbers[850];


    // ---------------
	// Zobrist methods
	// ---------------

    void Zobrist::generate(const Board& board)
    {
        // To generate hash from scratch, we simply test position for every indyvidual hashing aspect
        m_hash = 0;

        // Piece placement hash
        for (int sq = 0; sq < SQUARE_RANGE; sq++) {
            Piece piece = board.on(Square(sq));
            if (piece != NO_PIECE)
                update(piece, Square(sq));
        }

        // Other aspects of the position
        if (board.side_to_move() == BLACK)
            update(BLACK);
        update(board.castling_rights());
        update(board.enpassant_square());
    }


    // --------------
	// Initialization
	// --------------

    void initialize_zobrist_numbers()
    {
        // This seed generates decent numbers
        constexpr unsigned int SEED = 410376;

        Random::StandardGenerator<Hash> generator(SEED);

        // We use a hash set to avoid duplications of hash key (each zobrist number must be distinctive from others)
        std::unordered_set<Hash> generated_codes;
        
        // Distinctive random number generation
        std::generate(ZobristNumbers, ZobristNumbers + 850, [&generated_codes, &generator]() {
            Hash code = 0;
            do
                code = generator.random();
            while (generated_codes.contains(code));

            // Save hash to not repeat it in the future
            generated_codes.insert(code);

            return code;
        });
    }

}