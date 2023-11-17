#include "evaluation.h"

namespace Evaluation {

    constexpr int COLLAPSED_PAWNS_MAX_ID = 256;
    constexpr float MAX_STAGE = 32.f;

    constexpr Bitboard FIANCHETTO_MASKS[COLOR_RANGE] = {
        0x0000000000244281,
        0x8142240000000000
    };

    bool DISTANT_PAWNS_CHECKS[COLLAPSED_PAWNS_MAX_ID] = { false };



    Value Evaluator::evaluate()
    {
        evaluation = 0;
        stage = board->gameStage();
        noPawns[WHITE] = Bitboards::popcount(board->pieces(WHITE, PAWN));
        noPawns[BLACK] = Bitboards::popcount(board->pieces(BLACK, PAWN));
        pawnAttacks[WHITE] = Pieces::pawnAttacks<WHITE>(board->pieces(WHITE, PAWN));
        pawnAttacks[BLACK] = Pieces::pawnAttacks<BLACK>(board->pieces(BLACK, PAWN));
        bishopExistence[WHITE][LIGHT_SQUARE] = board->pieces(WHITE, LIGHT_SQUARE, BISHOP);
        bishopExistence[WHITE][DARK_SQUARE] = board->pieces(WHITE, DARK_SQUARE, BISHOP);
        bishopExistence[BLACK][LIGHT_SQUARE] = board->pieces(BLACK, LIGHT_SQUARE, BISHOP);
        bishopExistence[BLACK][DARK_SQUARE] = board->pieces(BLACK, DARK_SQUARE, BISHOP);

        evaluation += evaluatePieces<WHITE>();
        evaluation -= evaluatePieces<BLACK>();

        return evaluation;
    }

    void Evaluator::initEvaluationTables()
    {
        // Game stage dependable tables
        for (int stage = 0; stage < GAME_STAGE_RANGE; stage++) {
            KNIGHT_DISTANT_PAWNS_PENALTY_INT[stage] = interpolate(param(KNIGHT_DISTANT_PAWNS_PENALTY),
                                                                  linearFunction, stage / MAX_STAGE);
            BISHOP_FIANCHETTO_BONUS_INT[stage] = interpolate(param(BISHOP_FIANCHETTO_BONUS), linearFunction, stage / MAX_STAGE);
            ROOK_UNDEVELOPED_PENALTY_INT[stage] = interpolate(param(ROOK_UNDEVELOPED_PENALTY), linearFunction, stage / MAX_STAGE);


            for (int pt = PAWN; pt <= QUEEN; pt++)
                PIECE_BASE_VALUES_INT[pt][stage] = interpolate(param(EvalParameters(PAWN_BASE_VALUE + pt - PAWN)),
                                                               linearFunction, stage / MAX_STAGE);
            
            Value colorWeaknessInt = interpolate(param(BISHOP_COLOR_WEAKNESS_BONUS), linearFunction, stage / MAX_STAGE);
            for (int squares = 0; squares < 6; squares++)
                BISHOP_COLOR_WEAKNESS_BONUS_INT[squares][stage] = interpolate(0, colorWeaknessInt, sigmoidMidAbrupt, squares / 5.f);

            for (int otp = I_DEG_OUTPOST; otp <= III_DEG_OUTPOST; otp++) {
                KNIGHT_OUTPOSTS_BONUSES_INT[otp][stage] = interpolate(param(EvalParameters(KNIGHT_OUTPOST_I_DEG_BONUS + otp)),
                                                                      linearFunction, stage / MAX_STAGE);
                BISHOP_OUTPOSTS_BONUSES_INT[otp][stage] = interpolate(param(EvalParameters(BISHOP_OUTPOST_I_DEG_BONUS + otp)),
                                                                      linearFunction, stage / MAX_STAGE);
            }

            Value knightNoMobilityInt = interpolate(param(KNIGHT_MOBILITY_ZERO), linearFunction, stage / MAX_STAGE);
            Value knightFullMobilityInt = interpolate(param(KNIGHT_MOBILITY_FULL), linearFunction, stage / MAX_STAGE);
            Value bishopNoMobilityInt = interpolate(param(BISHOP_MOBILITY_ZERO), linearFunction, stage / MAX_STAGE);
            Value bishopFullMobilityInt = interpolate(param(BISHOP_MOBILITY_FULL), linearFunction, stage / MAX_STAGE);
            Value rookNoMobilityInt = interpolate(param(ROOK_MOBILITY_ZERO), linearFunction, stage / MAX_STAGE);
            Value rookFullMobilityInt = interpolate(param(ROOK_MOBILITY_FULL), linearFunction, stage / MAX_STAGE);
            Value queenNoMobilityInt = interpolate(param(QUEEN_MOBILITY_ZERO), linearFunction, stage / MAX_STAGE);
            Value queenFullMobilityInt = interpolate(param(QUEEN_MOBILITY_FULL), linearFunction, stage / MAX_STAGE);
            for (int mob = 0; mob < 9; mob++)
                KNIGHT_MOBILITY_INT[mob][stage] = interpolate(knightNoMobilityInt, knightFullMobilityInt, squareRootFunction, mob / 8.f);
            for (int mob = 0; mob < 14; mob++)
                BISHOP_MOBILITY_INT[mob][stage] = interpolate(bishopNoMobilityInt, bishopFullMobilityInt, squareRootFunction, mob / 13.f);
            for (int mob = 0; mob < 15; mob++)
                ROOK_MOBILITY_INT[mob][stage] = interpolate(rookNoMobilityInt, rookFullMobilityInt, squareRootFunction, mob / 14.f);
            for (int mob = 0; mob < 28; mob++)
                QUEEN_MOBILITY_INT[mob][stage] = interpolate(queenNoMobilityInt, queenFullMobilityInt, squareRootFunction, mob / 27.f);
        }

        // Tables not dependable on game stage
        BISHOP_PAIR_BONUS_INT = param(BISHOP_PAIR_BONUS).opening;
        ROOK_ON_SEMIOPENFILE_BONUS_INT = param(ROOK_ON_SEMIOPEN_FILE_BONUS).opening;
        ROOK_ON_OPEN_FILE_BONUS_INT = param(ROOK_ON_OPEN_FILE_BONUS).opening;
        ROOK_ON_78_RANK_BONUS_INT = param(ROOK_ON_78_RANK_BONUS).opening;
        for (int pawns = 0; pawns < 17; pawns++)
            KNIGHT_PAWNS_BONUS_INT[pawns] = interpolate(0, param(KNIGHT_PAWNS_BONUS).opening, quadraticFunction, pawns / 16.f);
        for (int pawns = 0; pawns < 9; pawns++)
            BAD_BISHOP_PENALTY_INT[pawns] = interpolate(0, param(BAD_BISHOP_PENALTY).opening, quadraticFunction, pawns / 8.f);

        // Other tables not contained by Evaluator
        for (int id = 1; id < COLLAPSED_PAWNS_MAX_ID; id++)
            DISTANT_PAWNS_CHECKS[id] = Bitboards::msb(Bitboard(id)) - Bitboards::lsb(Bitboard(id)) > 4;
    }



    template <Color side>
    Value Evaluator::evaluatePawns()
    {
        Bitboard pawnsBB = board->pieces(side, PAWN);

        return Bitboards::popcount(pawnsBB) * PIECE_BASE_VALUES_INT[PAWN][stage];
    }

    template Value Evaluator::evaluatePawns<WHITE>();
    template Value Evaluator::evaluatePawns<BLACK>();


    template <Color side>
    Value Evaluator::evaluatePieces()
    {
        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
        constexpr Bitboard rank4 = (side == WHITE ? ROW_4 : ROW_5);
        constexpr Bitboard ranks78 = (side == WHITE ? (ROW_7 | ROW_8) : (ROW_1 | ROW_2));

        Value result = 0;
        Bitboard potentialOutposts = pawnAttacks[side] & (~pawnAttacks[enemy]);


        // Knights
        Bitboard knightsBB = board->pieces(side, KNIGHT);
        int noKnights = Bitboards::popcount(knightsBB);
        int collapsedPawnsID = Bitboards::collapseFiles(board->pieces(enemy, PAWN));
        increaseValue<side, true>(result, noKnights * PIECE_BASE_VALUES_INT[KNIGHT][stage], "Knight base value");
        increaseValue<side, true>(result, noKnights * KNIGHT_PAWNS_BONUS_INT[noPawns[WHITE] + noPawns[BLACK]], "Knight pawns bonus");
        if (DISTANT_PAWNS_CHECKS[collapsedPawnsID])
            increaseValue<side, true>(result, noKnights * KNIGHT_DISTANT_PAWNS_PENALTY_INT[stage], "Knight distant pawns penalty");
        while (knightsBB) {
            Square sq = Bitboards::popLsb(knightsBB);
            Bitboard sqBB = squareToBB(sq);
            Bitboard attacks = Pieces::knightAttacks(sq) & (~board->pieces(side));

            // Outposts
            if (potentialOutposts & sqBB) {
                Bitboard pawnsArea = Bitboards::verticalShift<forwardDir, false>(adjacentRankSquares(sq));
                pawnsArea = Bitboards::verticalFill<forwardDir>(pawnsArea);
                if (!(pawnsArea & board->pieces(enemy, PAWN))) {
                    if (!board->pieces(enemy, KNIGHT) && !bishopExistence[enemy][colorOf(sq)])
                        increaseValue<side, true>(result, KNIGHT_OUTPOSTS_BONUSES_INT[III_DEG_OUTPOST][stage], "Knight outpost III deg");
                    else
                        increaseValue<side, true>(result, KNIGHT_OUTPOSTS_BONUSES_INT[II_DEG_OUTPOST][stage], "Knight outpost II deg");
                }
                else
                    increaseValue<side, true>(result, KNIGHT_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage], "Knight outpost I deg");
            }
            else if (potentialOutposts & attacks)
                increaseValue<side, true>(result, KNIGHT_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage] >> 1, "Knight attack on outpost");
            
            // Mobility
            Bitboard safeMoves = attacks & (~pawnAttacks[enemy]);
            increaseValue<side, true>(result, KNIGHT_MOBILITY_INT[Bitboards::popcount(safeMoves)][stage], "Knight mobility");
        }


        // Bishops
        Bitboard bishopsBB = board->pieces(side, BISHOP);
        Bitboard pawnsFrontFill = Bitboards::verticalFill<forwardDir>(board->pieces(side, PAWN) | rank4);
        increaseValue<side, true>(result, Bitboards::popcount(bishopsBB) * PIECE_BASE_VALUES_INT[BISHOP][stage], "Bishop base value");
        if (bishopExistence[side][LIGHT_SQUARE] && bishopExistence[side][DARK_SQUARE])
            increaseValue<side, true>(result, BISHOP_PAIR_BONUS_INT, "Bishop pair");
        while (bishopsBB) {
            Square sq = Bitboards::popLsb(bishopsBB);
            Bitboard sqBB = squareToBB(sq);

            // Own pawns on square same colored as bishop
            if (pawnsFrontFill & sqBB)
                increaseValue<side, true>(result, BAD_BISHOP_PENALTY_INT[noPawns[side]] >> 2, "Bad (but not that bad) bishop");
            else
                increaseValue<side, true>(result, BAD_BISHOP_PENALTY_INT[noPawns[side]], "Bad bishop");

            // Fianchetto
            if (sqBB & FIANCHETTO_MASKS[side])
                increaseValue<side, true>(result, BISHOP_FIANCHETTO_BONUS_INT[stage], "Bishop fianchetto");

            // Color weakness
            SquareColor bishopColor = colorOf(sq);
            if (!bishopExistence[enemy][bishopColor]) {
                Square enemyKingPos = board->kingPosition(enemy);
                Bitboard weakSquares = (Pieces::kingAttacks(enemyKingPos) | enemyKingPos) &
                                       squaresOfColor(bishopColor) & (~board->pieces(enemy, PAWN));
                increaseValue<side, true>(result, BISHOP_COLOR_WEAKNESS_BONUS_INT[Bitboards::popcount(weakSquares)][stage], "Color weakness");
            }

            // Outposts
            Bitboard attacks = Pieces::bishopAttacks(sq, board->pieces());
            if (potentialOutposts & sqBB) {
                Bitboard pawnsArea = Bitboards::verticalShift<forwardDir, false>(adjacentRankSquares(sq));
                pawnsArea = Bitboards::verticalFill<forwardDir>(pawnsArea);
                if (!(pawnsArea & board->pieces(enemy, PAWN))) {
                    if (!board->pieces(enemy, KNIGHT) && !bishopExistence[enemy][bishopColor])
                        increaseValue<side, true>(result, BISHOP_OUTPOSTS_BONUSES_INT[III_DEG_OUTPOST][stage], "Bishop outpost III deg");
                    else
                        increaseValue<side, true>(result, BISHOP_OUTPOSTS_BONUSES_INT[II_DEG_OUTPOST][stage], "Bishop outpost II deg");
                }
                else
                    increaseValue<side, true>(result, BISHOP_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage], "Bishop outpost I deg");
            }
            else if (potentialOutposts & attacks)
                increaseValue<side, true>(result, BISHOP_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage] >> 1, "Bishop attack on outpost");

            // Mobility
            Bitboard safeMoves = attacks & (~pawnAttacks[enemy]);
            increaseValue<side, true>(result, BISHOP_MOBILITY_INT[Bitboards::popcount(safeMoves)][stage], "Bishop mobility");
        }


        // Rooks
        Bitboard rooksBB = board->pieces(side, ROOK);
        increaseValue<side, true>(result, Bitboards::popcount(rooksBB) * PIECE_BASE_VALUES_INT[ROOK][stage], "Rook base value");
        while (rooksBB) {
            Square sq = Bitboards::popLsb(rooksBB);
            Bitboard sqBB = squareToBB(sq);

            // Semiopen, open files and development penalty
            Bitboard fileForward = Bitboards::verticalFill<forwardDir>(sqBB);
            if (!(fileForward & board->pieces(side, PAWN))) {
                if (!(fileForward & board->pieces(enemy, PAWN)))
                    increaseValue<side, true>(result, ROOK_ON_OPEN_FILE_BONUS_INT, "Rook on open file");
                else
                    increaseValue<side, true>(result, ROOK_ON_SEMIOPENFILE_BONUS_INT, "Rook on semiopen file");
            }
            else if (PATHS_TO_CENTRAL_FILES[sq] & board->pieces(side))
                increaseValue<side, true>(result, ROOK_UNDEVELOPED_PENALTY_INT[stage], "Undeveloped rook");
            
            // Rooks on 7-8 rank
            if (ranks78 & sqBB)
                increaseValue<side, true>(result, ROOK_ON_78_RANK_BONUS_INT, "Rook on 7/8 rank");

            // Mobility
            Bitboard safeMoves = Pieces::rookAttacks(sq, board->pieces()) & (~pawnAttacks[enemy]);
            increaseValue<side, true>(result, ROOK_MOBILITY_INT[Bitboards::popcount(safeMoves)][stage], "Rook mobility");
        }


        // Queens
        Bitboard queensBB = board->pieces(side, QUEEN);
        increaseValue<side, true>(result, Bitboards::popcount(queensBB) * PIECE_BASE_VALUES_INT[QUEEN][stage], "Queen base value");
        while (queensBB) {
            Square sq = Bitboards::popLsb(queensBB);

            // Mobility
            Bitboard safeMoves = Pieces::queenAttacks(sq, board->pieces()) & (~pawnAttacks[enemy]);
            increaseValue<side, true>(result, QUEEN_MOBILITY_INT[Bitboards::popcount(safeMoves)][stage], "Queen mobility");
        }

        return result;
    }

    template Value Evaluator::evaluatePieces<WHITE>();
    template Value Evaluator::evaluatePieces<BLACK>();


}