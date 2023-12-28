#pragma once

#include "evaluationConfig.h"
#include "../logic/boardConfig.h"
#include <iostream>


namespace Evaluation {

    // Hyperparameters
    constexpr int ATTACK_WEIGHTS[PIECE_TYPE_RANGE] = { 0, 0, 2, 2, 3, 5, 0, 0 };

    constexpr int DEFENSE_WEIGHTS[PIECE_TYPE_RANGE] = { 0, 0, 3, 3, 2, 1, 0, 0 };

    constexpr int THREAT_VALUES[PIECE_TYPE_RANGE][PIECE_TYPE_RANGE] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 25, 27, 29, 33, 0, 0},
        {0, 0, 0, 17, 24, 30, 0, 0},
        {0, 0, 15, 0, 23, 29, 0, 0},
        {0, 0, 12, 14, 0, 24, 0, 0},
        {0, 0, 7, 5, 0, 0, 0, 0},
        {0, 0, 6, 5, 0, 6, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
    };

    constexpr int PAWN_DEFENDING_WEIGHT = 2;
    constexpr int PAWN_ATTACKING_WEIGHT = 3;
    constexpr int PASSED_PAWN_SUPPORTING_WEIGHT = 10;
    constexpr int PASSED_PAWN_STOPPING_WEIGHT = 8;

    constexpr int ENDGAME_MARK = 8;

    constexpr float OPPOSITE_COLOR_BISHOPS_FACTOR[ENDGAME_MARK + 1] = {
        0.8f, 0.75f, 0.7f, 0.65f, 0.6f, 0.55f, 0.5f, 0.f, 0.f
    };
    constexpr float ROOKS_ENDGAME_FACTOR[ENDGAME_MARK + 1] = {
        1.f, 0.75f, 0.7f, 0.65f, 0.6f, 0.5f, 1.f, 1.f, 1.f
    };



    class Evaluator
    {
    public:
        Evaluator(BoardConfig* board) : board(board) { initEvaluationTables(); }
        ~Evaluator() { delete KING_AREA_ATTACKS_VALUE_INT; }

        Value evaluate();

        template <Color side>
        int threats() const;

    private:
        void initEvaluationTables();

        template <Color side>
        void initSideData();
        template <Color side>
        Value evaluatePawns();
        template <Color side>
        Value evaluatePieces();
        template <Color side>
        Value evaluateKing();
        template <Color side>
        Value evaluateOtherFeatures();
        Value adjustWinningChances();

        template <Color side>
        bool bishopPair() const;
        bool oppositeColorBishops() const;
        template <Color side>
        void updatePawnProximity(Square pawnSq, int ourWeight, int enemyWeight);
        template <Color side, PieceType type>
        void updateAttackDefenseTables(Bitboard attacks);
        template <Color side, PieceType type>
        void updateThreatsTables(Bitboard threatenedPieces);
        template <Color side>
        Value fileSafetyEval(Bitboard fileBB);

        BoardConfig* board;

        // Calculations shared among multiple evaluation parts
        Value pieceEvaluation = 0;
        Value kingEvaluation = 0;
        std::uint16_t stage = GAME_STAGE_MAX_VALUE;

        int noPieces[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };
        Bitboard pieceAttacks[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };
        Bitboard multipleAttacks[COLOR_RANGE] = { 0 };
        bool bishopExistence[COLOR_RANGE][SQUARE_COLOR_RANGE] = { false };
        Bitboard kingArea[COLOR_RANGE] = { 0 };
        Bitboard kingUpperArea[COLOR_RANGE] = { 0 };
        Bitboard kingUnsafeArea[COLOR_RANGE] = { 0 };
        Bitboard passedPawns[COLOR_RANGE] = { 0 };

        Value passerValues[SQUARE_RANGE] = { 0 };
        int pawnStormPoints[COLOR_RANGE] = { 0 };
        int kingAttackersCount[COLOR_RANGE] = { 0 };
        int kingAttackersPoints[COLOR_RANGE] = { 0 };
        int kingDefendersCount[COLOR_RANGE] = { 0 };
        int kingDefendersPoints[COLOR_RANGE] = { 0 };
        int threatCount[COLOR_RANGE] = { 0 };
        Value threatPoints[COLOR_RANGE] = { 0 };
        int pawnProximityDistances[COLOR_RANGE] = { 0 };
        int pawnProximityWeights[COLOR_RANGE] = { 0 };

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
        Value ISOLATED_PAWN_BASE_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value ISOLATED_PAWN_ATTACKED_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value DOUBLED_PAWN_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value BACKWARD_PAWN_BASE_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value BACKWARD_PAWN_ATTACKED_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value HANGING_PAWN_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value PASSED_PAWN_RANK_BONUS_INT[7][GAME_STAGE_RANGE] = { 0 };
        Value CONNECTED_PASSER_BONUS_INT[GAME_STAGE_RANGE] = { 0 };
        Value PAWN_SHIELD_STRONG_BONUS_INT[GAME_STAGE_RANGE] = { 0 };
        Value PAWN_SHIELD_WEAKER_BONUS_INT[GAME_STAGE_RANGE] = { 0 };
        Value PAWN_STORM_PENALTY_INT[17][GAME_STAGE_RANGE] = { 0 };
        Value SEMIOPEN_FILE_NEAR_KING_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value OPEN_FILE_NEAR_KING_PENALTY_INT[GAME_STAGE_RANGE] = { 0 };
        Value KING_CENTER_DISTANCE_VALUE_INT[7][GAME_STAGE_RANGE] = { 0 };
        Value KING_AREA_ATTACKS_MAX_POINTS_INT = 0;
        Value* KING_AREA_ATTACKS_VALUE_INT = nullptr;
        Value KING_PAWN_PROXIMITY_VALUE_INT[GAME_STAGE_RANGE] = { 0 };
        Value SPACE_BONUS_INT[GAME_STAGE_RANGE] = { 0 };
        Value UNCONTESTED_SPACE_BONUS_INT[GAME_STAGE_RANGE] = { 0 };
    };


    template <Color side>
    inline int Evaluator::threats() const
    {
        return threatCount[side];
    }

    template <Color side>
    inline bool Evaluator::bishopPair() const
    {
        return bishopExistence[side][LIGHT_SQUARE] && bishopExistence[side][DARK_SQUARE];
    }

    inline bool Evaluator::oppositeColorBishops() const
    {
        return noPieces[WHITE][BISHOP] == 1 && 
               noPieces[BLACK][BISHOP] == 1 &&
               bishopExistence[WHITE][LIGHT_SQUARE] != bishopExistence[BLACK][LIGHT_SQUARE];
    }

    template <Color side>
    inline void Evaluator::updatePawnProximity(Square pawnSq, int ourWeight, int enemyWeight)
    {
        constexpr Color enemy =  ~side;
        pawnProximityDistances[side] += ourWeight * Pieces::kingDistance(board->kingPosition(side), pawnSq);
        pawnProximityWeights[side] += ourWeight;
        pawnProximityDistances[enemy] += enemyWeight * Pieces::kingDistance(board->kingPosition(enemy), pawnSq);
        pawnProximityWeights[enemy] += enemyWeight;
    }

    template <Color side, PieceType type>
    inline void Evaluator::updateAttackDefenseTables(Bitboard attacks)
    {
        constexpr Color enemy = ~side;
        int attacksOnEnemyKing = Bitboards::popcount(attacks & kingUnsafeArea[enemy]);
        if (attacksOnEnemyKing > 0) {
            kingAttackersCount[enemy]++;
            kingAttackersPoints[enemy] += attacksOnEnemyKing * ATTACK_WEIGHTS[type];
        }
        int defenseCount = Bitboards::popcount(attacks & kingArea[side]);
        if (defenseCount > 1 && (attacks & kingUpperArea[side])) {
            kingDefendersCount[side]++;
            kingDefendersPoints[side] += defenseCount * DEFENSE_WEIGHTS[type];
        }
    }

    template <Color side, PieceType type>
    inline void Evaluator::updateThreatsTables(Bitboard threatenedPieces)
    {
        while (threatenedPieces) {
            Square sq = Bitboards::popLsb(threatenedPieces);
            threatPoints[side] += THREAT_VALUES[type][typeOf(board->onSquare(sq))];
            threatCount[side]++;
        }
    }

    template <Color side>
    inline Value Evaluator::fileSafetyEval(Bitboard fileBB)
    {
        constexpr Color enemy = ~side;
        int heavyPieceDiff = Bitboards::popcount(fileBB & board->pieces(enemy, ROOK, QUEEN)) - 
                             Bitboards::popcount(fileBB & board->pieces(side, ROOK, QUEEN));
        if (heavyPieceDiff > 0) {
            if (!(fileBB & board->pieces(PAWN)))
                return heavyPieceDiff * OPEN_FILE_NEAR_KING_PENALTY_INT[stage];
            else if (!(fileBB & board->pieces(enemy, PAWN)))
                return heavyPieceDiff * SEMIOPEN_FILE_NEAR_KING_PENALTY_INT[stage];
        }
        return 0;
    }



    // Temporary function for debugging purposes
    template <Color side, bool show>
    inline void increaseValue(Value& value, Value incr, const char* featureName)
    {
        if constexpr (show)
            std::cout << "Feature \"" << featureName << "\" evaluated for side " << side << " as: " << incr << std::endl;
        value += incr;
    }

}