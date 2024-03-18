#pragma once

#include <cinttypes>
#include "types.h"

class BoardConfig;


namespace Zobrist {

    using U64 = ::std::uint64_t;

    int hammingDistance(U64 code1, U64 code2);
    float averageHammingDistance(const U64 codes[], int n);
    int minimalHammingDistance(const U64 codes[], int n);

    void initZobristHashing();

    constexpr int HASH_CODES_NUM = 850;
    constexpr int PIECE_CODE_RANGE[PIECE_RANGE] = {
        -1, 0, 64, 128, 196, 256, 320, -1,
        -1, 384, 448, 512, 576, 640, 704, -1
    };
    constexpr int CASTLING_HASH_CODES_BEGIN = 768;
    constexpr int ENPASSANT_HASH_CODES_BEGIN = 784;
    constexpr int SIDE_ON_MOVE_HAS_CODES_BEGIN = 849;
    
    extern U64 hashCodes[HASH_CODES_NUM];


    // -----------------------------
    // Zobrist container and methods
    // -----------------------------

    class ZobristHash
    {
    public:
        ZobristHash() : hash(0ULL) {}

        void generateHash(BoardConfig* board);
        void updateByPlacementChange(Piece piece, Square sq);
        void updateByCastlingRightsChange(int rights);
        void updateByEnpassantChange(Square epSquare);
        void updateBySideOnMoveChange();

        U64 getHash() const;
        void restoreHash(U64 hash);

    private:
        U64 hash;
    };


    inline U64 ZobristHash::getHash() const
    {
        return hash;
    }

    inline void ZobristHash::restoreHash(U64 hash)
    {
        this->hash = hash;
    }

    inline void ZobristHash::updateByPlacementChange(Piece piece, Square sq)
    {
        hash ^= hashCodes[PIECE_CODE_RANGE[piece] + sq];
    }

    inline void ZobristHash::updateByCastlingRightsChange(int rights)
    {
        hash ^= hashCodes[CASTLING_HASH_CODES_BEGIN + rights];
    }

    inline void ZobristHash::updateByEnpassantChange(Square epSquare)
    {
        hash ^= hashCodes[ENPASSANT_HASH_CODES_BEGIN + epSquare];
    }

    inline void ZobristHash::updateBySideOnMoveChange()
    {
        hash ^= hashCodes[SIDE_ON_MOVE_HAS_CODES_BEGIN];
    }

}