#pragma once

#include "evaluationConfig.h"
#include "../logic/boardConfig.h"
#include <iostream>


namespace Evaluation {

    // Temporary function for debugging purposes
    template <Color side, bool show>
    inline void increaseValue(Value& value, Value incr, const std::string& featureName)
    {
        if constexpr (show)
            std::cout << "Feature \"" << featureName << "\" evaluated for side " << side << " as: " << incr << std::endl;
        value += incr;
    }


    class Evaluator
    {
    public:
        Evaluator(BoardConfig* board) : board(board) {}

        Value evaluate();

    private:
        void initEvaluationTables();
        template <Color side>
        Value evaluatePawns();
        template <Color side>
        Value evaluatePieces();

        BoardConfig* board;

        // Calculations shared among multiple evaluation parts
        Value evaluation = 0;
        Value materialBalance = 0;
        int stage = GAME_STAGE_MAX_VALUE;
        
        int noPawns[COLOR_RANGE] = { 0 };
        Bitboard pawnAttacks[COLOR_RANGE] = { 0 };
        bool bishopExistence[COLOR_RANGE][SQUARE_COLOR_RANGE] = { false };

        // Evaluation tables
        Value PIECE_BASE_VALUES_INT[PIECE_TYPE_RANGE][GAME_STAGE_RANGE] = { 0 };
        Value KNIGHT_PAWNS_BONUS_INT[17] = { 0 };
        Value KNIGHT_DISTANT_PAWNS_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value KNIGHT_OUTPOSTS_BONUSES_INT[OUTPOST_TYPE_RANGE][GAME_STAGE_RANGE] = { 0 };
        Value KNIGHT_MOBILITY_INT[9][GAME_STAGE_RANGE] = { 0 };
        Value BISHOP_PAIR_BONUS_INT = 0;
        Value BAD_BISHOP_PENALTY_INT[9] = { 0 };
        Value BISHOP_FIANCHETTO_BONUS_INT[GAME_STAGE_RANGE] = { 0 };
        Value BISHOP_COLOR_WEAKNESS_BONUS_INT[6][GAME_STAGE_RANGE] = { 0 };
        Value BISHOP_OUTPOSTS_BONUSES_INT[OUTPOST_TYPE_RANGE][GAME_STAGE_RANGE] = { 0 };
        Value BISHOP_MOBILITY_INT[14][GAME_STAGE_RANGE] = { 0 };
        Value ROOK_ON_SEMIOPENFILE_BONUS_INT = 0;
        Value ROOK_ON_OPEN_FILE_BONUS_INT = 0;
        Value ROOK_ON_78_RANK_BONUS_INT = 0;
        Value ROOK_UNDEVELOPED_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value ROOK_MOBILITY_INT[15][GAME_STAGE_RANGE] = { 0 };
        Value QUEEN_MOBILITY_INT[28][GAME_STAGE_RANGE] = { 0 };
    };

}