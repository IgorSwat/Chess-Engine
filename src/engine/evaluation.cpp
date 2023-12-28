#include "evaluation.h"
#include <algorithm>

namespace Evaluation {

    constexpr int COLLAPSED_PAWNS_MAX_ID = 256;
    constexpr float MAX_STAGE = 32.f;
    constexpr float MAX_STORM_POINTS = 16.f;

    // Testing flags
    constexpr bool knightShow = false;
    constexpr bool bishopShow = false;
    constexpr bool rookShow = false;
    constexpr bool queenShow = false;
    constexpr bool basicShow = false;
    constexpr bool passedShow = false;
    constexpr bool kingShow = false;
    constexpr bool spaceShow = false;
    constexpr bool threatsShow = false;

    // Precalculations
    constexpr Bitboard FIANCHETTO_MASKS[COLOR_RANGE] = {
        0x0000000000244281,
        0x8142240000000000
    };

    constexpr int DISTANCE_TO_CENTER[SQUARE_RANGE] = {
        6, 5, 4, 3, 3, 4, 5, 6,
        5, 4, 3, 2, 2, 3, 4, 5,
        4, 3, 2, 1, 1, 2, 3, 4,
        3, 2, 1, 0, 0, 1, 2, 3,
        3, 2, 1, 0, 0, 1, 2, 3,
        4, 3, 2, 1, 1, 2, 3, 4,
        5, 4, 3, 2, 2, 3, 4, 5,
        6, 5, 4, 3, 3, 4, 5, 6
    };

    bool DISTANT_PAWNS_CHECKS[COLLAPSED_PAWNS_MAX_ID] = { false };



    void Evaluator::initEvaluationTables()
    {
        // Game stage dependable tables
        for (int stage = 0; stage < GAME_STAGE_RANGE; stage++) {
            KNIGHT_DISTANT_PAWNS_PENALTY_INT[stage] = interpolate(param(KNIGHT_DISTANT_PAWNS_PENALTY),
                                                                  linearFunction, stage / MAX_STAGE);
            BISHOP_FIANCHETTO_BONUS_INT[stage] = interpolate(param(BISHOP_FIANCHETTO_BONUS), linearFunction, stage / MAX_STAGE);
            ROOK_UNDEVELOPED_PENALTY_INT[stage] = interpolate(param(ROOK_UNDEVELOPED_PENALTY), linearFunction, stage / MAX_STAGE);
            ISOLATED_PAWN_BASE_PENALTY_INT[stage] = interpolate(param(ISOLATED_PAWN_BASE_PENALTY), linearFunction, stage / MAX_STAGE);
            ISOLATED_PAWN_ATTACKED_PENALTY_INT[stage] = interpolate(param(ISOLATED_PAWN_ATTACKED_PENALTY), 
                                                                    linearFunction, stage / MAX_STAGE);
            DOUBLED_PAWN_PENALTY_INT[stage] = interpolate(param(DOUBLED_PAWN_PENALTY), linearFunction, stage / MAX_STAGE);
            BACKWARD_PAWN_BASE_PENALTY_INT[stage] = interpolate(param(BACKWARD_PAWN_BASE_PENALTY),
                                                                linearFunction, stage / MAX_STAGE);
            BACKWARD_PAWN_ATTACKED_PENALTY_INT[stage] = interpolate(param(BACKWARD_PAWN_ATTACKED_PENALTY), 
                                                                    linearFunction, stage / MAX_STAGE);
            HANGING_PAWN_PENALTY_INT[stage] = interpolate(param(HANGING_PAWN_PENALTY), linearFunction, stage / MAX_STAGE);
            CONNECTED_PASSER_BONUS_INT[stage] = interpolate(param(CONNECTED_PASSER_BONUS), linearFunction, stage / MAX_STAGE);
            PAWN_SHIELD_STRONG_BONUS_INT[stage] = interpolate(param(PAWN_SHIELD_STRONG_BONUS), linearFunction, stage / MAX_STAGE);
            PAWN_SHIELD_WEAKER_BONUS_INT[stage] = interpolate(param(PAWN_SHIELD_WEAKER_BONUS), linearFunction, stage / MAX_STAGE);
            SEMIOPEN_FILE_NEAR_KING_PENALTY_INT[stage] = interpolate(param(SEMIOPEN_FILE_NEAR_KING_PENALTY), linearFunction, stage / MAX_STAGE);
            OPEN_FILE_NEAR_KING_PENALTY_INT[stage] = interpolate(param(OPEN_FILE_NEAR_KING_PENALTY), linearFunction, stage /  MAX_STAGE);
            KING_PAWN_PROXIMITY_VALUE_INT[stage] = interpolate(param(KING_PAWN_PROXIMITY_VALUE), linearFunction, stage / MAX_STAGE);
            SPACE_BONUS_INT[stage] = interpolate(param(SPACE_BONUS), linearFunction, stage / MAX_STAGE);
            UNCONTESTED_SPACE_BONUS_INT[stage] = interpolate(param(UNCONTESTED_SPACE_BONUS), linearFunction, stage / MAX_STAGE);


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
            
            Value passerBonus = interpolate(param(PASSED_PAWN_MAX_BONUS), linearFunction, stage / MAX_STAGE);
            for (int dist = 1; dist < 7; dist++) {
                PASSED_PAWN_RANK_BONUS_INT[dist][stage] = passerBonus;
                passerBonus >>= 1;
            }

            Value pawnStormMaxValue = interpolate(param(PAWN_STORM_PENALTY), linearFunction, stage / MAX_STAGE);
            for (int pts = 0; pts < 17; pts++)
                PAWN_STORM_PENALTY_INT[pts][stage] = interpolate(0, pawnStormMaxValue, sigmoidLowAbrupt, pts / MAX_STORM_POINTS);
            
            Value kingCenterDistanceValue = interpolate(param(KING_CENTER_DISTANCE_MIN_VALUE), 
                                                        [](float x){return x >= 1.f / 4.f ? 4.f * x / 3.f - 1.f / 3.f: 0.f;}, stage / MAX_STAGE);
            for (int dist = 0; dist < 7; dist++)
                KING_CENTER_DISTANCE_VALUE_INT[dist][stage] = interpolate(0, kingCenterDistanceValue, highDegPolynomial, (6 - dist) / 6.f);
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
        KING_AREA_ATTACKS_MAX_POINTS_INT = param(KING_AREA_ATTACKS_MAX_POINTS).opening;
        KING_AREA_ATTACKS_VALUE_INT = new Value[KING_AREA_ATTACKS_MAX_POINTS_INT + 1];
        for (int pts = 0; pts <= KING_AREA_ATTACKS_MAX_POINTS_INT; pts++)
            KING_AREA_ATTACKS_VALUE_INT[pts] = interpolate(0, param(KING_AREA_ATTACKS_MAX_VALUE).opening,
                                                           sigmoidLowAbrupt, float(pts) / KING_AREA_ATTACKS_MAX_POINTS_INT);

        // Other tables not contained by Evaluator
        for (int id = 1; id < COLLAPSED_PAWNS_MAX_ID; id++)
            DISTANT_PAWNS_CHECKS[id] = Bitboards::msb(Bitboard(id)) - Bitboards::lsb(Bitboard(id)) > 4;
    }


    Value Evaluator::evaluate()
    {
        pieceEvaluation = kingEvaluation = 0;
        stage = board->gameStage();

        initSideData<WHITE>();
        initSideData<BLACK>();

        pieceEvaluation += evaluatePieces<WHITE>();
        pieceEvaluation -= evaluatePieces<BLACK>();
        pieceEvaluation += evaluatePawns<WHITE>();
        pieceEvaluation -= evaluatePawns<BLACK>();
        kingEvaluation += evaluateKing<WHITE>();
        kingEvaluation -= evaluateKing<BLACK>();
        pieceEvaluation += evaluateOtherFeatures<WHITE>();
        pieceEvaluation -= evaluateOtherFeatures<BLACK>();

        return adjustWinningChances();
    }


    template <Color side>
    void Evaluator::initSideData()
    {
        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);

        const Square kingPos = board->kingPosition(side);
        const Bitboard doublePawnAttacks = Pieces::doublePawnAttacks<side>(board->pieces(side, PAWN));

        noPieces[side][PAWN] = Bitboards::popcount(board->pieces(side, PAWN));

        pieceAttacks[side][PAWN] = Pieces::pawnAttacks<side>(board->pieces(side, PAWN));
        pieceAttacks[side][KING] = Pieces::pieceAttacks<KING>(kingPos);
        pieceAttacks[side][KNIGHT] = 0;
        pieceAttacks[side][BISHOP] = 0;
        pieceAttacks[side][ROOK] = 0;
        pieceAttacks[side][QUEEN] = 0;
        pieceAttacks[side][ALL_PIECES] = pieceAttacks[side][PAWN] | pieceAttacks[side][KING];
        multipleAttacks[side] = (pieceAttacks[side][PAWN] & pieceAttacks[side][KING]) | doublePawnAttacks;

        bishopExistence[side][LIGHT_SQUARE] = board->pieces(side, LIGHT_SQUARE, BISHOP);
        bishopExistence[side][DARK_SQUARE] = board->pieces(side, DARK_SQUARE, BISHOP);

        pawnStormPoints[side] = 0;

        kingArea[side] = Pieces::pieceAttacks<KING>(kingPos) | kingPos;
        kingArea[side] |= Bitboards::shift<forwardDir>(kingArea[side]);
        kingUpperArea[side] = kingArea[side] ^ adjacentRankSquares(kingPos) ^ kingPos;

        kingAttackersCount[side] = Bitboards::popcount(board->pieces(enemy, PAWN) & Bitboards::shift<forwardDir>(kingArea[side]));
        kingAttackersPoints[side] = 0;
        kingDefendersCount[side] = kingDefendersPoints[side] = 0;

        kingUnsafeArea[side] = kingArea[side] & (~doublePawnAttacks);

        threatCount[side] = 0;
        threatPoints[side] = 0;

        pawnProximityDistances[side] = 0;
        pawnProximityWeights[side] = 0;

        passedPawns[side] = 0;
    }

    template void Evaluator::initSideData<WHITE>();
    template void Evaluator::initSideData<BLACK>();


    template <Color side>
    Value Evaluator::evaluatePawns()
    {
        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
        constexpr Direction backwardDir = (side == WHITE ? SOUTH : NORTH);
        constexpr Bitboard ourHalfOfBoard = (side == WHITE ? 0x00000000ffffffff : 0xffffffff00000000);

        Value result = 0;
        Bitboard doubledPawns = 0;
        Bitboard passers = 0;
        Bitboard pawnsBB = board->pieces(side, PAWN);

        const Square enemyKingPos = board->kingPosition(enemy);
        const Bitboard enemyKingFrontArea = Bitboards::verticalFill<backwardDir>(adjacentRankSquares(enemyKingPos) | enemyKingPos);
        const Bitboard blockedPawns = pawnsBB & Bitboards::shift<backwardDir>(board->pieces(enemy, PAWN));

        increaseValue<side, basicShow>(result, noPieces[side][PAWN] * PIECE_BASE_VALUES_INT[PAWN][stage], "Pawns base value");
        while (pawnsBB) {
            Square sq = Bitboards::popLsb(pawnsBB);
            Bitboard sqBB = squareToBB(sq);
            Bitboard file = fileBBOf(sq);
            Bitboard pawnArea = adjacentRankSquares(sq) | sqBB;
            Bitboard frontSpawn = Bitboards::shift<forwardDir>(sqBB);
            Bitboard frontArea = Bitboards::verticalFill<forwardDir>(Bitboards::shift<forwardDir>(pawnArea));
            Bitboard backArea = Bitboards::verticalFill<backwardDir>(pawnArea) ^ sqBB;
            Bitboard fileFrontArea = frontArea & file;
            Bitboard fileBackArea = backArea & file;

            // Backwards, isolated and other hanging pawns
            if (!(backArea & board->pieces(side, PAWN)) && (pieceAttacks[enemy][PAWN] & frontSpawn) && (ourHalfOfBoard & sqBB)) {
                increaseValue<side, basicShow>(result, BACKWARD_PAWN_BASE_PENALTY_INT[stage], "Backward pawn");
                if (multipleAttacks[enemy] & sqBB)
                    increaseValue<side, basicShow>(result, BACKWARD_PAWN_ATTACKED_PENALTY_INT[stage] << 1, "Backward pawn under attack");
                else if (pieceAttacks[enemy][ALL_PIECES] & sqBB)
                    increaseValue<side, basicShow>(result, BACKWARD_PAWN_ATTACKED_PENALTY_INT[stage], "Backward pawn under attack");
                updatePawnProximity<side>(sq, PAWN_DEFENDING_WEIGHT, PAWN_ATTACKING_WEIGHT);
            }
            else if (!(adjacentFiles(sq) & board->pieces(side, PAWN))) {
                increaseValue<side, basicShow>(result, ISOLATED_PAWN_BASE_PENALTY_INT[stage], "Isolated pawn");
                if (multipleAttacks[enemy] & sqBB)
                    increaseValue<side, basicShow>(result, ISOLATED_PAWN_ATTACKED_PENALTY_INT[stage] << 1, "Isolated pawn under attack");
                else if (pieceAttacks[enemy][ALL_PIECES] & sqBB)
                    increaseValue<side, basicShow>(result, ISOLATED_PAWN_ATTACKED_PENALTY_INT[stage], "Isolated pawn under attack");
                updatePawnProximity<side>(sq, PAWN_DEFENDING_WEIGHT, PAWN_ATTACKING_WEIGHT);
            }
            else if (!(pieceAttacks[side][PAWN] & sqBB)) {
                increaseValue<side, basicShow>(result, HANGING_PAWN_PENALTY_INT[stage], "Hanging pawn");
                updatePawnProximity<side>(sq, PAWN_DEFENDING_WEIGHT, PAWN_ATTACKING_WEIGHT);
            }

            // Doubled pawns
            if (board->pieces(side, PAWN) & fileBackArea) {
                doubledPawns |= sqBB;
                if (doubledPawns & fileBackArea)
                    increaseValue<side, basicShow>(result, DOUBLED_PAWN_PENALTY_INT[stage] << 2, "Tripled (or worse) pawn");
                else if (board->pieces(enemy, PAWN) & (frontArea ^ fileFrontArea))
                    increaseValue<side, basicShow>(result, DOUBLED_PAWN_PENALTY_INT[stage] >> 1, "(A little bit better) doubled pawn");
                else
                    increaseValue<side, basicShow>(result, DOUBLED_PAWN_PENALTY_INT[stage], "Doubled pawn");
            }

            // Passed pawns 1/3
            if (!(frontArea & board->pieces(enemy, PAWN)) && !(fileFrontArea & board->pieces(side, PAWN))) {
                passers |= sqBB;
                int distanceToPromotion = side == WHITE ? 7 - rankOf(sq) : rankOf(sq);
                passerValues[sq] = PASSED_PAWN_RANK_BONUS_INT[distanceToPromotion][stage];
                updatePawnProximity<side>(sq, PASSED_PAWN_SUPPORTING_WEIGHT, PASSED_PAWN_STOPPING_WEIGHT);
            }

            // Pawn storms 1/2
            if ((enemyKingFrontArea & sqBB) && !(blockedPawns & sqBB)) {
                int distance = side == WHITE ? rankOf(enemyKingPos) - rankOf(sq) : rankOf(sq) - rankOf(enemyKingPos);
                pawnStormPoints[enemy] += 7 - distance;
            }
        }

        // Passed pawns 2/3
        passedPawns[side] = passers;
        Bitboard passedPawnsBB = passers;
        Bitboard supportedPassers = 0;
        while (passedPawnsBB) {
            Square sq = Bitboards::popLsb(passedPawnsBB);
            Bitboard sqBB = squareToBB(sq);
            bool isSupported = false;

            // Blockers and attacks on front spawn
            Square frontSpawn = sq + forwardDir;
            Piece blocker = board->onSquare(frontSpawn);
            bool diagonalBlocker = false;
            if (blocker != NO_PIECE && colorOf(blocker) == side) {      // 3/4 of full bonus
                passerValues[sq] *= 3;
                passerValues[sq] >>= 2;
            }
            else if (blocker != NO_PIECE) {                             // 1/2 of full bonus
                passerValues[sq] >>= 1;
                PieceType type = typeOf(blocker);
                diagonalBlocker = type == BISHOP || type == QUEEN || type == KING;
            }
            else if (multipleAttacks[enemy] & frontSpawn) {             // 5/8 of full bonus
                passerValues[sq] *= 5;
                passerValues[sq] >>= 3;
            }
            else if (pieceAttacks[enemy][ALL_PIECES] & frontSpawn) {    // 3/4 of full bonus
                passerValues[sq] *= 3;
                passerValues[sq] >>= 2;
            }

            // Left file connected passers
            Bitboard leftFilePassers = Bitboards::shift<WEST>(fileBBOf(sq)) & passers;
            if (leftFilePassers) {
                Square connectedPasserPos = Bitboards::lsb(leftFilePassers);
                Bitboard connectedPasserPosBB = squareToBB(connectedPasserPos);
                if (!(supportedPassers & sqBB)) {
                    passerValues[sq] <<= 1;
                    passerValues[sq] += CONNECTED_PASSER_BONUS_INT[stage];
                }
                if (!(supportedPassers & connectedPasserPosBB)) {
                    passerValues[connectedPasserPos] <<= 1;
                    passerValues[connectedPasserPos] += CONNECTED_PASSER_BONUS_INT[stage];
                }
                if (diagonalBlocker)
                    passerValues[connectedPasserPos] >>= 1;
                supportedPassers |= sqBB;
                supportedPassers |= connectedPasserPosBB;
            }

            // Right file connected passers
            Bitboard rightFilePassers = Bitboards::shift<EAST>(fileBBOf(sq)) & passers;
            if (rightFilePassers && diagonalBlocker) {
                Square connectedPasserPos = Bitboards::lsb(rightFilePassers);
                passerValues[connectedPasserPos] >>= 1;
            }

            // Protected passed pawn
            if (!leftFilePassers && !rightFilePassers && (pieceAttacks[side][PAWN] & sqBB))
                passerValues[sq] <<= 1;
        }

        while (passers) {
            Square sq = Bitboards::popLsb(passers);
            increaseValue<side, passedShow>(result, passerValues[sq], "Passed pawn");
        }

        return result;
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
        constexpr Bitboard ourHalfOfBoard = (side == WHITE ? 0x00000000ffffffff : 0xffffffff00000000);
        constexpr Bitboard enemyHalfOfBoard = (side == WHITE ? 0xffffffff00000000 : 0x00000000ffffffff);

        Value result = 0;
        const Bitboard potentialOutposts = enemyHalfOfBoard & pieceAttacks[side][PAWN] & (~pieceAttacks[enemy][PAWN]);
        const Bitboard safeSquares = ~(board->pieces(side) | pieceAttacks[enemy][PAWN]);


        // Knights
        Bitboard knightsBB = board->pieces(side, KNIGHT);
        noPieces[side][KNIGHT] = Bitboards::popcount(knightsBB);
        int collapsedPawnsID = Bitboards::collapseFiles(board->pieces(enemy, PAWN));
        increaseValue<side, knightShow>(result, noPieces[side][KNIGHT] * PIECE_BASE_VALUES_INT[KNIGHT][stage], 
                                        "Knight base value");
        increaseValue<side, knightShow>(result, noPieces[side][KNIGHT] * KNIGHT_PAWNS_BONUS_INT[noPieces[WHITE][PAWN] + noPieces[BLACK][PAWN]],
                                        "Knight pawns bonus");
        if (DISTANT_PAWNS_CHECKS[collapsedPawnsID])
            increaseValue<side, knightShow>(result, noPieces[side][KNIGHT] * KNIGHT_DISTANT_PAWNS_PENALTY_INT[stage],
                                            "Knight distant pawns penalty");
        while (knightsBB) {
            Square sq = Bitboards::popLsb(knightsBB);
            Bitboard sqBB = squareToBB(sq);
            Bitboard attacks = Pieces::pieceAttacks<KNIGHT>(sq);
            Bitboard safeMoves = attacks & safeSquares;

            multipleAttacks[side] |= attacks & pieceAttacks[side][ALL_PIECES];
            pieceAttacks[side][KNIGHT] |= attacks;
            pieceAttacks[side][ALL_PIECES] |= attacks;
            
            updateAttackDefenseTables<side, KNIGHT>(attacks);

            // Outposts
            if (potentialOutposts & sqBB) {
                Bitboard pawnsArea = Bitboards::shift<forwardDir>(adjacentRankSquares(sq));
                pawnsArea = Bitboards::verticalFill<forwardDir>(pawnsArea);
                if (!(pawnsArea & board->pieces(enemy, PAWN))) {
                    if (!board->pieces(enemy, KNIGHT) && !bishopExistence[enemy][colorOf(sq)])
                        increaseValue<side, knightShow>(result, KNIGHT_OUTPOSTS_BONUSES_INT[III_DEG_OUTPOST][stage], "Knight outpost III deg");
                    else
                        increaseValue<side, knightShow>(result, KNIGHT_OUTPOSTS_BONUSES_INT[II_DEG_OUTPOST][stage], "Knight outpost II deg");
                }
                else
                    increaseValue<side, knightShow>(result, KNIGHT_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage], "Knight outpost I deg");
            }
            else if (potentialOutposts & safeMoves)
                increaseValue<side, knightShow>(result, KNIGHT_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage] >> 1, "Knight attack on outpost");
            
            // Mobility
            int noSafeMoves = Bitboards::popcount(safeMoves);
            if (board->pinnedPieces(side) & sqBB)
                noSafeMoves >>= 1;
            increaseValue<side, knightShow>(result, KNIGHT_MOBILITY_INT[noSafeMoves][stage], "Knight mobility");
        }


        // Bishops
        Bitboard bishopsBB = board->pieces(side, BISHOP);
        noPieces[side][BISHOP] = Bitboards::popcount(bishopsBB);
        Bitboard pawnsFrontFill = Bitboards::verticalFill<forwardDir>(board->pieces(side, PAWN) | rank4);
        int noPawnsOnColors[SQUARE_COLOR_RANGE] = { Bitboards::popcount(board->pieces(side, DARK_SQUARE, PAWN)),
                                                      Bitboards::popcount(board->pieces(side, LIGHT_SQUARE, PAWN)) };
        increaseValue<side, bishopShow>(result, noPieces[side][BISHOP] * PIECE_BASE_VALUES_INT[BISHOP][stage], "Bishop base value");
        if (bishopPair<side>())
            increaseValue<side, bishopShow>(result, BISHOP_PAIR_BONUS_INT, "Bishop pair");
        while (bishopsBB) {
            Square sq = Bitboards::popLsb(bishopsBB);
            Bitboard sqBB = squareToBB(sq);
            SquareColor bishopColor = colorOf(sq);
            Bitboard attacks = Pieces::pieceAttacks<BISHOP>(sq, board->pieces());
            Bitboard xRayAttacks = Pieces::xRayBishopAttacks(sq, board->pieces(), board->pieces(side, BISHOP, QUEEN));
            Bitboard allAttacks = attacks | xRayAttacks;
            Bitboard safeMoves = attacks & safeSquares;

            multipleAttacks[side] |= allAttacks & pieceAttacks[side][ALL_PIECES];
            pieceAttacks[side][BISHOP] |= attacks;
            pieceAttacks[side][ALL_PIECES] |= attacks;

            updateAttackDefenseTables<side, BISHOP>(allAttacks);

            // Fianchetto & bad bishop
            if (sqBB & FIANCHETTO_MASKS[side]) {
                increaseValue<side, bishopShow>(result, BISHOP_FIANCHETTO_BONUS_INT[stage], "Bishop fianchetto");
                Bitboard longDiagonal = bishopColor == DARK_SQUARE ? DIAG_A1H8 : DIAG_A8H1;
                if (longDiagonal & board->pieces(side, PAWN)) {     // Fianchetto bishop is considered bad only if there are friendly pawns blocking the long diagonal
                    if (pawnsFrontFill & sqBB)
                        increaseValue<side, bishopShow>(result, BAD_BISHOP_PENALTY_INT[noPawnsOnColors[bishopColor]] >> 2, "Bad (but not that bad) fianchetto bishop");
                    else
                        increaseValue<side, bishopShow>(result, BAD_BISHOP_PENALTY_INT[noPawnsOnColors[bishopColor]], "Bad fianchetto bishop");
                }
            }
            else {
                if (pawnsFrontFill & sqBB)
                    increaseValue<side, bishopShow>(result, BAD_BISHOP_PENALTY_INT[noPawnsOnColors[bishopColor]] >> 2, "Bad (but not that bad) bishop");
                else
                    increaseValue<side, bishopShow>(result, BAD_BISHOP_PENALTY_INT[noPawnsOnColors[bishopColor]], "Bad bishop");
            }

            // Color weakness
            if (!bishopExistence[enemy][bishopColor]) {
                Square enemyKingPos = board->kingPosition(enemy);
                Bitboard weakSquares = Pieces::pieceAttacks<KING>(enemyKingPos) & squaresOfColor(bishopColor) & (~board->pieces(enemy, PAWN));
                increaseValue<side, bishopShow>(result, BISHOP_COLOR_WEAKNESS_BONUS_INT[Bitboards::popcount(weakSquares)][stage], "Color weakness");
            }

            // Outposts
            if (potentialOutposts & sqBB) {
                Bitboard pawnsArea = Bitboards::shift<forwardDir>(adjacentRankSquares(sq));
                pawnsArea = Bitboards::verticalFill<forwardDir>(pawnsArea);
                if (!(pawnsArea & board->pieces(enemy, PAWN))) {
                    if (!board->pieces(enemy, KNIGHT) && !bishopExistence[enemy][bishopColor])
                        increaseValue<side, bishopShow>(result, BISHOP_OUTPOSTS_BONUSES_INT[III_DEG_OUTPOST][stage], "Bishop outpost III deg");
                    else
                        increaseValue<side, bishopShow>(result, BISHOP_OUTPOSTS_BONUSES_INT[II_DEG_OUTPOST][stage], "Bishop outpost II deg");
                }
                else
                    increaseValue<side, bishopShow>(result, BISHOP_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage], "Bishop outpost I deg");
            }
            else if (potentialOutposts & safeMoves)
                increaseValue<side, bishopShow>(result, BISHOP_OUTPOSTS_BONUSES_INT[I_DEG_OUTPOST][stage] >> 1, "Bishop attack on outpost");

            // Mobility
            int noSafeMoves = Bitboards::popcount(safeMoves);
            if (board->pinnedPieces(side) & sqBB)
                noSafeMoves >>= 1;
            increaseValue<side, bishopShow>(result, BISHOP_MOBILITY_INT[noSafeMoves][stage], "Bishop mobility");
        }


        // Rooks
        Bitboard rooksBB = board->pieces(side, ROOK);
        noPieces[side][ROOK] = Bitboards::popcount(rooksBB);
        increaseValue<side, rookShow>(result, noPieces[side][ROOK] * PIECE_BASE_VALUES_INT[ROOK][stage], "Rook base value");
        while (rooksBB) {
            Square sq = Bitboards::popLsb(rooksBB);
            Bitboard sqBB = squareToBB(sq);
            Bitboard attacks = Pieces::pieceAttacks<ROOK>(sq, board->pieces());
            Bitboard xRayAttacks = Pieces::xRayRookAttacks(sq, board->pieces(), board->pieces(side, ROOK, QUEEN));
            Bitboard allAttacks = attacks | xRayAttacks;
            Bitboard safeMoves = attacks & safeSquares;

            multipleAttacks[side] |= allAttacks & pieceAttacks[side][ALL_PIECES];
            pieceAttacks[side][ROOK] |= attacks;
            pieceAttacks[side][ALL_PIECES] |= attacks;

            updateAttackDefenseTables<side, ROOK>(allAttacks);

            // Semiopen, open files and development penalty
            Bitboard fileForward = Bitboards::verticalFill<forwardDir>(sqBB);
            if (!(fileForward & board->pieces(side, PAWN))) {
                if (!(fileForward & board->pieces(enemy, PAWN)))
                    increaseValue<side, rookShow>(result, ROOK_ON_OPEN_FILE_BONUS_INT, "Rook on open file");
                else
                    increaseValue<side, rookShow>(result, ROOK_ON_SEMIOPENFILE_BONUS_INT, "Rook on semiopen file");
            }
            else if (PATHS_TO_CENTRAL_FILES[sq] & board->pieces(side, KING))
                increaseValue<side, rookShow>(result, ROOK_UNDEVELOPED_PENALTY_INT[stage], "Undeveloped rook");
            
            // Rooks on 7-8 rank
            if (ranks78 & sqBB)
                increaseValue<side, rookShow>(result, ROOK_ON_78_RANK_BONUS_INT, "Rook on 7/8 rank");

            // Mobility
            int noSafeMoves = Bitboards::popcount(safeMoves);
            if (board->pinnedPieces(side) & sqBB)
                noSafeMoves >>= 1;
            increaseValue<side, rookShow>(result, ROOK_MOBILITY_INT[noSafeMoves][stage], "Rook mobility");
        }


        // Queens
        Bitboard queensBB = board->pieces(side, QUEEN);
        noPieces[side][QUEEN] = Bitboards::popcount(queensBB);
        increaseValue<side, queenShow>(result, noPieces[side][QUEEN] * PIECE_BASE_VALUES_INT[QUEEN][stage], "Queen base value");
        while (queensBB) {
            Square sq = Bitboards::popLsb(queensBB);
            Bitboard attacks = Pieces::pieceAttacks<QUEEN>(sq, board->pieces());
            Bitboard xRayAttacks = Pieces::xRayQueenAttacks(sq, board->pieces(), board->pieces(side, BISHOP, ROOK, QUEEN));
            Bitboard allAttacks = attacks | xRayAttacks;
            Bitboard safeMoves = attacks & safeSquares;

            multipleAttacks[side] |= allAttacks & pieceAttacks[side][ALL_PIECES];
            pieceAttacks[side][QUEEN] |= attacks;
            pieceAttacks[side][ALL_PIECES] |= attacks;

            updateAttackDefenseTables<side, QUEEN>(allAttacks);

            // Mobility
            int noSafeMoves = Bitboards::popcount(safeMoves);
            if (board->pinnedPieces(side) & sq)
                noSafeMoves >>= 1;
            increaseValue<side, queenShow>(result, QUEEN_MOBILITY_INT[noSafeMoves][stage], "Queen mobility");
        }

        return result;
    }

    template Value Evaluator::evaluatePieces<WHITE>();
    template Value Evaluator::evaluatePieces<BLACK>();


    template <Color side>
    Value Evaluator::evaluateKing()
    {
        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);

        Value result = 0;
        Square sq = board->kingPosition(side);
        Bitboard sqBB = squareToBB(sq);
        Bitboard rankArea = adjacentRankSquares(sq) | sqBB;

        // Pawn shield
        Bitboard frontRankArea = Bitboards::shift<forwardDir>(rankArea);
        Bitboard frontShield = frontRankArea & board->pieces(side, PAWN);
        increaseValue<side, kingShow>(result, Bitboards::popcount(frontShield) * PAWN_SHIELD_STRONG_BONUS_INT[stage], "Frontal pawn shield");
        Bitboard otherShielders = (Bitboards::shift<forwardDir>(frontRankArea) | adjacentRankSquares(sq)) & board->pieces(side, PAWN);
        increaseValue<side, kingShow>(result, Bitboards::popcount(otherShielders) * PAWN_SHIELD_WEAKER_BONUS_INT[stage], "Other shields");

        // Pawn storms 2/2
        increaseValue<side, kingShow>(result, PAWN_STORM_PENALTY_INT[std::min(pawnStormPoints[side], 16)][stage], "Pawn storm");

        // Semiopen & open files
        Bitboard centralFile = fileBBOf(sq);
        increaseValue<side, kingShow>(result, fileSafetyEval<side>(centralFile), "(Semi)open file near king");
        increaseValue<side, kingShow>(result, (3 * fileSafetyEval<side>(Bitboards::shift<WEST>(centralFile))) >> 2, "(Semi)open file near king");
        increaseValue<side, kingShow>(result, (3 * fileSafetyEval<side>(Bitboards::shift<EAST>(centralFile))) >> 2, "(Semi)open file near king");

        // Distance from center
        int centerDistance = DISTANCE_TO_CENTER[sq];
        increaseValue<side, kingShow>(result, KING_CENTER_DISTANCE_VALUE_INT[centerDistance][stage], "King distance to center");

        // Pawn proximity
        float weightedAverage = static_cast<float>(pawnProximityDistances[side]) / pawnProximityWeights[side] - 1.f;
        Value proximityValue = static_cast<Value>(weightedAverage * KING_PAWN_PROXIMITY_VALUE_INT[stage]);
        increaseValue<side, kingShow>(result, proximityValue, "King and pawns proximity");

        // King area attacks and defense
        Value attackDefenseID = std::max(1, 1 + kingAttackersCount[side] - kingDefendersCount[side]) *
                              std::max(kingAttackersPoints[side] >> 1, kingAttackersPoints[side] - kingDefendersPoints[side]);
        attackDefenseID = std::min(attackDefenseID, KING_AREA_ATTACKS_MAX_POINTS_INT);
        increaseValue<side, kingShow>(result, KING_AREA_ATTACKS_VALUE_INT[attackDefenseID], "Attacks on king area");

        return result;
    }

    template Value Evaluator::evaluateKing<WHITE>();
    template Value Evaluator::evaluateKing<BLACK>();


    template <Color side>
    Value Evaluator::evaluateOtherFeatures()
    {
        constexpr Color enemy = ~side;
        constexpr Direction backwardDir = (side == WHITE ? SOUTH : NORTH);
        constexpr Bitboard sideArea = (side == WHITE ? ROW_2 | ROW_3 | ROW_4 : ROW_7 | ROW_6 | ROW_5);

        Value result = 0;

        // Space
        Value spaceIIIdeg = 0;  // Central files
        Value spaceIIdeg = 0;   // B and G files
        Value spaceIdeg = 0;    // A and H files

        Bitboard behindPawns = Bitboards::verticalFill<backwardDir>(Bitboards::shift<backwardDir>(board->pieces(side, PAWN)));
        Bitboard safeSquares = sideArea & behindPawns & (~pieceAttacks[enemy][PAWN]) & (~board->pieces(side));
        Bitboard uncontestedSquares = safeSquares & (~pieceAttacks[enemy][ALL_PIECES]);
        safeSquares ^= uncontestedSquares;
        spaceIIIdeg += Bitboards::popcount(safeSquares & CENTRAL_FILES) * SPACE_BONUS_INT[stage];
        spaceIIIdeg += Bitboards::popcount(uncontestedSquares & CENTRAL_FILES) * UNCONTESTED_SPACE_BONUS_INT[stage];
        spaceIIdeg += Bitboards::popcount(safeSquares & BG_FILES) * SPACE_BONUS_INT[stage];
        spaceIIdeg += Bitboards::popcount(uncontestedSquares & BG_FILES) * UNCONTESTED_SPACE_BONUS_INT[stage];
        spaceIIdeg >>= 1;
        spaceIdeg += Bitboards::popcount(safeSquares & EDGE_FILES) * SPACE_BONUS_INT[stage];
        spaceIdeg += Bitboards::popcount(uncontestedSquares & EDGE_FILES) * SPACE_BONUS_INT[stage];
        spaceIdeg >>= 2;

        increaseValue<side, spaceShow>(result, spaceIIIdeg + spaceIIdeg + spaceIdeg, "Space");

        // Threats & connectivity
        Bitboard safePieces = board->pieces(enemy, KNIGHT, BISHOP, ROOK, QUEEN);
        Bitboard undefendedSquares = ~pieceAttacks[enemy][ALL_PIECES];
        
        Bitboard pawnThreats = safePieces & pieceAttacks[side][PAWN];
        safePieces ^= pawnThreats;
        updateThreatsTables<side, PAWN>(pawnThreats);
        Bitboard knightThreats = ((board->pieces(enemy, BISHOP) & undefendedSquares) |
                                   board->pieces(enemy, ROOK, QUEEN)) & safePieces & pieceAttacks[side][KNIGHT];
        safePieces ^= knightThreats;
        updateThreatsTables<side, KNIGHT>(knightThreats);
        Bitboard bishopThreats = ((board->pieces(enemy, KNIGHT) & undefendedSquares) |
                                   board->pieces(enemy, ROOK, QUEEN)) & safePieces & pieceAttacks[side][BISHOP];
        safePieces ^= bishopThreats;
        updateThreatsTables<side, BISHOP>(bishopThreats);
        Bitboard rookThreats = ((safePieces & board->pieces(enemy, KNIGHT, BISHOP) & undefendedSquares) |
                                 board->pieces(enemy, QUEEN)) & safePieces & pieceAttacks[side][ROOK];
        safePieces ^= rookThreats;
        updateThreatsTables<side, ROOK>(rookThreats);
        Bitboard queenKingThreats = safePieces & undefendedSquares & (pieceAttacks[side][QUEEN] | pieceAttacks[side][KING]);
        threatCount[side] += Bitboards::popcount(queenKingThreats);

        if (threatCount[side] != 1 && (threatCount[side] & 0x1))
            increaseValue<side, threatsShow>(result, threatPoints[side] * (threatCount[side] - 1), "Threats");
        else
            increaseValue<side, threatsShow>(result, threatPoints[side] * threatCount[side], "Threats");

        // Tempo bonus
        if (board->movingSide() == side)
            result += 28;
        
        return result;
    }

    template Value Evaluator::evaluateOtherFeatures<WHITE>();
    template Value Evaluator::evaluateOtherFeatures<BLACK>();


    // This function takes the calculated values of pieceEvaluation and kingEvaluation
    // and adjust them considering winning chances in given position, returns the final evaluation of the position
    Value Evaluator::adjustWinningChances()
    {
        // Consider most common types of difficult endgames to win
        if (stage <= ENDGAME_MARK) {
            if (oppositeColorBishops()) {       // Opposite color bishops endgame
                float factor = OPPOSITE_COLOR_BISHOPS_FACTOR[stage];
                pieceEvaluation *= factor;
            }
            else if (!board->pieces(KNIGHT, BISHOP) && !passedPawns[WHITE] && !passedPawns[BLACK]) {        // Rook endgame
                float factor = ROOKS_ENDGAME_FACTOR[stage];
                pieceEvaluation *= factor;
            }
            else if (pieceEvaluation > 0 && noPieces[WHITE][PAWN] == 0)
                pieceEvaluation >>= 1;
            else if (pieceEvaluation < 0 && noPieces[BLACK][PAWN] == 0)
                pieceEvaluation >>= 1;
        }

        int totalEvaluation = pieceEvaluation + kingEvaluation;

        // Flattening the eval with no progress determined by halfmove clock
        int halfmoveClock = board->halfmovesClocked();
        totalEvaluation *= (100 - halfmoveClock);
        totalEvaluation /= 100;

        return Value(totalEvaluation);
    }

}