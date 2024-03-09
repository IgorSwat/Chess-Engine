#include "evaluation.h"
#include <algorithm>

namespace Evaluation {

    // --------------------------
    // Evaluation hyperparameters
    // --------------------------

    constexpr int MAX_PIECE_DENSITY = 16;

    // Proximity factors
    constexpr int OUR_PAWN_PROXIMITY_FACTOR = 2;
    constexpr int ENEMY_PAWN_PROXIMITY_FACTOR = 3;
    constexpr int OUR_PASSER_PROXIMITY_FACTOR = 8;
    constexpr int ENEMY_PASSER_PROXIMITY_FACTOR = 12;

    // Pawn structure points
    constexpr int GOOD_PAWN = -4;
    constexpr int UNDEFENDED_MOBILE_PAWN = 1;
    constexpr int DEFENDED_BLOCKED_PAWN = 2;
    constexpr int ISOLATED_MOBILE_PAWN = 7;
    constexpr int UNDEFENDED_BLOCKED_PAWN = 8;
    constexpr int ISOLATED_BLOCKED_PAWN = 15;
    constexpr int BACKWARD_PAWN = 15;


    // -------------------------------------
    // Piece-square tables & other constants
    // -------------------------------------

    constexpr int OutpostFactors[COLOR_RANGE][SQUARE_RANGE] = {

        // For white side
        {0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         1, 2, 2, 3, 3, 2, 2, 1,
         2, 4, 7, 10, 10, 7, 4, 2,
         3, 8, 13, 14, 14, 13, 8, 3,
         4, 10, 15, 16, 16, 15, 10, 4,
         3, 9, 11, 13, 13, 11, 9, 3,
         2, 4, 5, 6, 6, 5, 4, 2},

         // For black side
        {2, 4, 5, 6, 6, 5, 4, 2,
         3, 9, 11, 13, 13, 11, 9, 3,
         4, 10, 15, 16, 16, 15, 10, 4,
         3, 8, 13, 14, 14, 13, 8, 3,
         2, 4, 7, 10, 10, 7, 4, 2,
         1, 2, 2, 3, 3, 2, 2, 1,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0}
    };

    constexpr int PawnShieldPoints[64] = {
        0, 10, 12, 38, 10, 28, 38, 64,
        2, 6, 32, 40, 24, 26, 60, 68,
        6, 30, 12, 44, 30, 50, 44, 72,
        12, 32, 34, 46, 38, 54, 50, 76,
        2, 24, 32, 60, 6, 26, 40, 68,
        6, 8, 34, 62, 8, 12, 62, 72,
        12, 38, 34, 50, 32, 54, 46, 76,
        16, 42, 36, 64, 42, 60, 64, 80
    };

    constexpr int PawnStormPoints[7] = { 
        0, -30, -28, -21, -11, -3, -1
    };

    constexpr int KingTropismKnightPoints = -28;

    constexpr int KingTropismBishopPoints[32] = {
        -20, -20, -17, -12, -9, -7, -6, -5,
        -5, -4, -4, -4, -4, -4, -4, -4,
        -4, -4, -4, -4, -4, -4, -4, -4,
        -4, -4, -4, -4, -4, -4, -4, -4
    };

    constexpr int KingTropismRookPoints[32] = {
        -32, -32, -27, -20, -14, -11, -9, -8,
        -7, -7, -6, -6, -6, -6, -6, -6,
        -6, -6, -6, -6, -6, -6, -6, -6,
        -6, -6, -6, -6, -6, -6, -6, -6
    };

    constexpr int KingTropismQueenPoints[32] = {
        -68, -68, -52, -35, -24, -21, -19, -17,
        -16, -15, -14, -14, -12, -12, -12, -12,
        -12, -12, -12, -12, -12, -12, -12, -12,
        -12, -12, -12, -12, -12, -12, -12, -12
    };

    constexpr int KingAreaAttackWages[PIECE_TYPE_RANGE] = {
        0,
        0,  // Pawn
        -6,  // Knight
        -6,  // Bishop
        -8,  // Rook
        -12,  // Queen
        0,  // King
        0
    };

    constexpr Bitboard FianchettoMap[COLOR_RANGE] = { 0x0000000000244281, 0x8142240000000000 };


    // ---------------------
    // Main Evaluation class
    // ---------------- ----

    void Evaluator::initEvalTables()
    {
        fill_params_table(KnightDensePosition, MAX_PIECE_DENSITY, parameters[KNIGHT_DENSE_POSITION_MIN], parameters[KNIGHT_DENSE_POSITION_MAX], Interpolation::linearFunction);
        fill_params_table(KnightPawnSpread, 7, parameters[KNIGHT_PAWN_SPREAD_COMP], EvalParameter(0, 0), [](float x) {return std::pow(x, 1.5); });
        fill_params_table(KnightMobility, 8, parameters[KNIGHT_MOBILITY_ZERO], parameters[KNIGHT_MOBILITY_FULL], Interpolation::reversedSigmoidLowAbrupt);
        fill_params_table(BishopOwnPawnBlockage, 6, parameters[BISHOP_OWN_PAWN_NO_BLOCKAGE], parameters[BISHOP_OWN_PAWN_FULL_BLOCKAGE], Interpolation::quadraticFunction);
        fill_params_table(BishopEnemyPawnBlockage, 6, parameters[BISHOP_ENEMY_PAWN_NO_BLOCKAGE], parameters[BISHOP_ENEMY_PAWN_FULL_BLOCKAGE], Interpolation::squareRootFunction);
        fill_params_table(BishopMobility, 13, parameters[BISHOP_MOBILITY_ZERO], parameters[BISHOP_MOBILITY_FULL], Interpolation::reversedSigmoidLowAbrupt);
        fill_params_table(RookMobility, 14, parameters[ROOK_MOBILITY_ZERO], parameters[ROOK_MOBILITY_FULL], Interpolation::reversedSigmoidLowAbrupt);
    }

    Value Evaluator::evaluate()
    {
        initCommonData<WHITE>();
        initCommonData<BLACK>();

        Value result = 0;
        result += evaluatePawns<WHITE>();
        result -= evaluatePawns<BLACK>();
        result += evaluatePieces<WHITE>();
        result -= evaluatePieces<BLACK>();
        result += evaluateKing<WHITE>();
        result -= evaluateKing<BLACK>();

        return result;
    }

    template <Color side>
    void Evaluator::initCommonData()
    {
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);

        stage = board->gameStage();

        // Piece-attack properties
        Bitboard pawns = board->pieces(side, PAWN);
        attacks[side][PAWN] = Pieces::pawn_attacks<side>(pawns);
        attacks[side][KING] = Pieces::piece_attacks_s<KING>(board->kingPosition(side));
        attacks[side][ALL_PIECES] = attacks[side][PAWN] | attacks[side][KING];

        // Pawn structure properties
        int fileIndex = Bitboards::fill<SOUTH>(board->pieces(PAWN)) & 0xff;
        pawnRankSpread = fileIndex ? file_of(Bitboards::msb(Bitboard(fileIndex))) - file_of(Bitboards::lsb(Bitboard(fileIndex))) : 0;
        mostAdvancedUnstopablePasser = 8;
        structurePoints[WHITE][LIGHT_SQUARE] = structurePoints[WHITE][DARK_SQUARE] = 0;
        structurePoints[BLACK][LIGHT_SQUARE] = structurePoints[BLACK][DARK_SQUARE] = 0;

        // King proximity
        proximityPoints[side] = proximityWages[side] = 0;

        // King safety related
        Square kingSq = board->kingPosition(side);
        safetyPoints[side] = 0;
        kingAreaAttackers[side] = 0;
        kingAreaAttackPoints[side] = 0;
        kingArea[side] = Pieces::piece_attacks_s<KING>(kingSq) | kingSq;
        kingFrontSpans[side][0] = Board::FrontSpan[kingSq][side] & kingArea[side];
        kingFrontSpans[side][1] = Bitboards::shift_s<forwardDir>(kingFrontSpans[side][0]);
        weakKingSpan[side] = kingFrontSpans[side][1] & (~Pieces::pawn_attacks_x2<side>(pawns));

        // Other common properties
        centralDensity = std::min(Bitboards::popcount(Board::CENTRAL_RANKS & board->pieces()), MAX_PIECE_DENSITY);
        boardDensity = Bitboards::popcount(board->pieces());
    }

    template void Evaluator::initCommonData<WHITE>();
    template void Evaluator::initCommonData<BLACK>();


    template <Color side>
    Value Evaluator::evaluatePawns()
    {
        Value result = 0;
        constexpr bool test = false;

        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
        constexpr int promotionRank = (side == WHITE ? 7 : 0);
        Bitboard ourPawns = board->pieces(side, PAWN);
        Bitboard enemyPawns = board->pieces(enemy, PAWN);
        Bitboard pawns = ourPawns;
        Bitboard enemyKingFront = Board::FrontSpan[board->kingPosition(enemy)][enemy];

        while (pawns) {
            Square sq = Bitboards::pop_lsb(pawns);
            Square front = sq + forwardDir;

            bool isDefendedByPawns = attacks[side][PAWN] & sq;
            bool isBlocked = board->pieces(PAWN) & front;
            bool isDoubled = Board::FrontSpan[sq][side] & Board::file_bb_of(sq) & ourPawns;

            // Pawn structure-wise strength
            if (isDefendedByPawns) {
                structurePoints[side][color_of(sq)] += isBlocked ? DEFENDED_BLOCKED_PAWN : GOOD_PAWN;
            }
            // Isolated pawn
            else if (!(Board::AdjacentFiles[sq] & ourPawns)) {
                structurePoints[side][color_of(sq)] += isBlocked ? ISOLATED_BLOCKED_PAWN : ISOLATED_MOBILE_PAWN;
                // ......
            }
            // Backward pawn
            else if (!(Board::back_span_m<side>(sq) & ourPawns) && ((enemyPawns | attacks[enemy][PAWN]) & front)) {
                structurePoints[side][color_of(sq)] += BACKWARD_PAWN;
                // ......
            }
            // Undefended, but neither isolated or backward
            else {
                structurePoints[side][color_of(sq)] += isBlocked ? UNDEFENDED_BLOCKED_PAWN : UNDEFENDED_MOBILE_PAWN;
            }

            // Passed pawn
            if (!(Board::FrontSpan[sq][side] & enemyPawns) && !isDoubled) {
                int promotionDistance = (side == WHITE ? 7 - rank_of(sq) : rank_of(sq));

                updateProximity<side>(sq, OUR_PASSER_PROXIMITY_FACTOR, ENEMY_PASSER_PROXIMITY_FACTOR);
                if (!stage && Board::SquareDistance[board->kingPosition(enemy)][make_square(promotionRank, file_of(sq))] - (board->movingSide() == enemy) > promotionDistance)
                    mostAdvancedUnstopablePasser = std::min(promotionDistance, mostAdvancedUnstopablePasser);
                else {
                    // Other passers calculations
                    // .......
                }
            }
            else if (!isDefendedByPawns)
                updateProximity<side>(sq, OUR_PAWN_PROXIMITY_FACTOR, ENEMY_PAWN_PROXIMITY_FACTOR);

            // Pawn storm
            if (enemyKingFront & sq) {
                int distance = side == WHITE ? rank_of(board->kingPosition(enemy)) - rank_of(sq) : rank_of(sq) - rank_of(board->kingPosition(enemy));
                safetyPoints[enemy] += isBlocked ? (PawnStormPoints[distance] >> 2) : PawnStormPoints[distance];
            }

            // King area attacks
            if (Pieces::pawn_attacks(side, sq) & kingArea[enemy])
                kingAreaAttackers[enemy]++;
        }
        
        // Unstopable passed pawn
        if (mostAdvancedUnstopablePasser < 8) {
            Value passerValue = ((8 - mostAdvancedUnstopablePasser) * parameters[QUEEN_BASE_VALUE].opening) >> 3;
            add_eval<side, test>(result, passerValue, "Unstopable passed pawn");
        }

        return result;
    }

    template Value Evaluator::evaluatePawns<WHITE>();
    template Value Evaluator::evaluatePawns<BLACK>();


    template <Color side>
    Value Evaluator::evaluatePieces()
    {
        Value material = 0;
        Value activity = 0;
        constexpr bool test = false;

        constexpr Color enemy = ~side;
        constexpr Bitboard rank_78 = (side == WHITE ? Board::RANK_7 | Board::RANK_8 : Board::RANK_1 | Board::RANK_2);
        Bitboard ourPawns = board->pieces(side, PAWN);
        Bitboard ourPieces = board->pieces(side) ^ ourPawns;
        Bitboard enemyPawns = board->pieces(enemy, PAWN);
        Bitboard enemyPieces = board->pieces(enemy) ^ enemyPawns;
        Bitboard ourPiecesExcluded = board->pieces() ^ ourPieces;
        Bitboard safeSquares = (Board::BOARD ^ (attacks[enemy][PAWN] | board->pieces(side))) | enemyPieces;
        Square enemyKingPos = board->kingPosition(enemy);

        // Knights
        Bitboard knights = board->pieces(side, KNIGHT);
        int noKnights = Bitboards::popcount(knights);

        add_eval<side, test>(material, parameters[KNIGHT_BASE_VALUE].opening * noKnights, "Knights material value");
        add_eval<side, test>(activity, KnightDensePosition[centralDensity].opening * noKnights, "Knights board density value");
        add_eval<side, test>(activity, Interpolation::interpolate_gs(KnightPawnSpread[pawnRankSpread], stage) * noKnights, "Knights pawn spread compensation");
        while (knights) {
            Square sq = Bitboards::pop_lsb(knights);
            Bitboard safetyBB = safeSquares & sq;
            Bitboard att = Pieces::piece_attacks_s<KNIGHT>(sq);

            int mobility = Bitboards::popcount(att & safeSquares);
            add_eval<side, test>(activity, Interpolation::interpolate_gs(KnightMobility[mobility], stage), "Knight mobility");

            // Outposts
            if (attacks[side][PAWN] & safeSquares) {
                // Possibility to attack outpost with pawn, II deg outpost
                if (enemyPawns & Board::FrontSpan[sq][side] & Board::AdjacentFiles[sq])
                    add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[KNIGHT_OUTPOST_II_DEG], stage) * OutpostFactors[side][sq]) >> 4, "Knight II deg outpost");
                // Uncontested, III deg outpost
                else
                    add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[KNIGHT_OUTPOST_III_DEG], stage) * OutpostFactors[side][sq]) >> 4, "Knight III deg outpost");
            }
            else if (attacks[side][ALL_PIECES] & safeSquares)
                add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[KNIGHT_OUTPOST_I_DEG], stage) * OutpostFactors[side][sq]) >> 4, "Knight I deg outpost");

            // King tropism
            int tropismDistance = (Board::SquareDistance[sq][enemyKingPos] - 1) >> 1;
            int tropism = KingTropismKnightPoints >> tropismDistance;
            safetyPoints[enemy] += tropism;

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, att, KingAreaAttackWages[KNIGHT]);
        }

        // Bishops
        Bitboard bishops = board->pieces(side, BISHOP);

        add_eval<side, test>(material, parameters[BISHOP_BASE_VALUE].opening * Bitboards::popcount(bishops), "Bishops material value");
        if ((bishops & Board::LIGHT_SQUARES) && (bishops & Board::DARK_SQUARES))
            add_eval<side, test>(activity, parameters[BISHOP_PAIR_BONUS].opening, "Bishop pair bonus");
        while (bishops) {
            Square sq = Bitboards::pop_lsb(bishops);
            SquareColor color = color_of(sq);
            Bitboard colorMap = (color == LIGHT_SQUARE ? Board::LIGHT_SQUARES : Board::DARK_SQUARES) & Board::NOT_EDGE_FILES;
            Bitboard xray = Pieces::xray_attacks<BISHOP>(sq, board->pieces(), board->pieces(side, BISHOP, QUEEN));

            // Our pawn blockage
            int ourBlockage = std::min(Bitboards::popcount(board->pieces(side, PAWN) & colorMap), 6);
            add_eval<side, test>(activity, Interpolation::interpolate_gs(BishopOwnPawnBlockage[ourBlockage], stage), "Own pawn placement (bishop)");

            // Enemy pawn blockage
            int enemyBlockage = std::min(Bitboards::popcount(enemyPawns & colorMap), 6);
            add_eval<side, test>(activity, Interpolation::interpolate_gs(BishopEnemyPawnBlockage[enemyBlockage], stage), "Enemy pawn placement (bishop)");

            // Enemy structure weakness
            Value structureWeaknessEval = (Interpolation::interpolate_gs(parameters[BISHOP_ENEMY_PAWN_WEAKNESS], stage) * structurePoints[enemy][color]) >> 4;
            add_eval<side, test>(activity, structureWeaknessEval, "Enemy pawn structure weakness (bishop)");

            // Mobility and fianchetto
            Bitboard occ = (FianchettoMap[side] & sq) ? board->pieces() ^ ourPieces : board->pieces();
            int mobility = Bitboards::popcount(Pieces::piece_attacks_s<BISHOP>(sq, occ) & safeSquares);
            add_eval<side, test>(activity, Interpolation::interpolate_gs(BishopMobility[mobility], stage), "Bishop mobility");

            // Outposts
            if (attacks[side][PAWN] & sq) {
                // Possibility to attack outpost with pawn, II deg outpost
                if (enemyPawns & Board::FrontSpan[sq][side] & Board::AdjacentFiles[sq])
                    add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[BISHOP_OUTPOST_II_DEG], stage) * OutpostFactors[side][sq]) >> 4, "Bishop II deg outpost");
                // Uncontested, III deg outpost
                else
                    add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[BISHOP_OUTPOST_III_DEG], stage) * OutpostFactors[side][sq]) >> 4, "Bishop III deg outpost");
            }
            else if (attacks[side][ALL_PIECES] & sq)
                add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[BISHOP_OUTPOST_I_DEG], stage) * OutpostFactors[side][sq]) >> 4, "Bishop I deg outpost");

            // King tropism
            if (Board::aligned(enemyKingPos, sq))
                safetyPoints[enemy] += KingTropismBishopPoints[Bitboards::popcount(Board::Paths[enemyKingPos][sq] & ourPiecesExcluded)];
            else {
                int distance = Board::SquareDistance[enemyKingPos][sq];
                Square boxCorner = rank_of(sq) >= rank_of(enemyKingPos) ?
                    (file_of(sq) >= file_of(enemyKingPos) ? make_square(std::min(rank_of(enemyKingPos) + distance, 7), std::min(file_of(enemyKingPos) + distance, 7)) :
                                                            make_square(std::min(rank_of(enemyKingPos) + distance, 7), std::max(file_of(enemyKingPos) - distance, 0))) :
                    (file_of(sq) >= file_of(enemyKingPos) ? make_square(std::max(rank_of(enemyKingPos) - distance, 0), std::min(file_of(enemyKingPos) + distance, 7)) :
                                                            make_square(std::max(rank_of(enemyKingPos) - distance, 0), std::max(file_of(enemyKingPos) - distance, 0)));
                int blockers = Bitboards::popcount(Board::Boxes[enemyKingPos][boxCorner] & ourPiecesExcluded & colorMap);
                safetyPoints[enemy] += (Pieces::pseudo_attacks(BISHOP, sq) & kingArea[enemy]) ? KingTropismBishopPoints[blockers] : KingTropismBishopPoints[blockers] >> 1;
            }

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, Pieces::piece_attacks_s<BISHOP>(sq, board->pieces()) | xray, KingAreaAttackWages[BISHOP]);
        }

        // Rooks
        Bitboard rooks = board->pieces(side, ROOK);
        int noRooks = Bitboards::popcount(rooks);
        int structureWeakness = structurePoints[enemy][LIGHT_SQUARE] + structurePoints[enemy][DARK_SQUARE];

        add_eval<side, test>(material, Interpolation::interpolate_gs(parameters[ROOK_BASE_VALUE], stage) * noRooks, "Rooks material value");
        add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[ROOK_ENEMY_PAWN_WEAKNESS], stage) * noRooks * structureWeakness) >> 5, "Enemy pawn structure weakness (rook)");
        while (rooks) {
            Square sq = Bitboards::pop_lsb(rooks);
            Bitboard fileBB = Board::file_bb_of(sq);
            Bitboard att = Pieces::piece_attacks_s<ROOK>(sq, board->pieces());
            Bitboard xray = Pieces::xray_attacks<ROOK>(sq, board->pieces(), board->pieces(side, ROOK, QUEEN));

            // Placement bonuses
            if (rank_78 & sq)
                add_eval<side, test>(activity, Interpolation::interpolate_gs(parameters[ROOK_ON_78_RANK_BONUS], stage), "Rook on 7-8 rank");
            else if (!(fileBB & board->pieces(PAWN)))
                add_eval<side, test>(activity, Interpolation::interpolate_gs(parameters[ROOK_ON_OPEN_FILE_BONUS], stage), "Rook on open file");
            else if (!(fileBB & ourPawns))
                add_eval<side, test>(activity, Interpolation::interpolate_gs(parameters[ROOK_ON_SEMIOPEN_FILE_BONUS], stage), "Rook on semiopen file");

            // Mobility
            int mobility = Bitboards::popcount(att & safeSquares);
            add_eval<side, test>(activity, Interpolation::interpolate_gs(RookMobility[mobility], stage), "Rook mobility");

            // King tropism
            int blockers = Bitboards::popcount(Board::Boxes[sq][enemyKingPos] & ourPiecesExcluded);
            int tropism = (Pieces::pseudo_attacks(ROOK, sq) & kingArea[enemy]) ? KingTropismRookPoints[blockers] : KingTropismRookPoints[blockers] >> 1;
            safetyPoints[enemy] += tropism;

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, att | xray, KingAreaAttackWages[ROOK]);
        }

        // Queens
        Bitboard queens = board->pieces(side, QUEEN);
        int noQueens = Bitboards::popcount(queens);

        add_eval<side, test>(material, parameters[QUEEN_BASE_VALUE].opening * noQueens, "Queens material value");
        add_eval<side, test>(activity, (Interpolation::interpolate_gs(parameters[QUEEN_ENEMY_PAWN_WEAKNESS], stage) * noQueens * structureWeakness) >> 5, "Enemy pawn structure weakness (queen)");
        while (queens) {
            Square sq = Bitboards::pop_lsb(queens);
            Bitboard att = Pieces::piece_attacks_s<QUEEN>(sq, board->pieces());
            Bitboard xray = Pieces::xray_attacks<QUEEN>(sq, board->pieces(), board->pieces(side, BISHOP, ROOK, QUEEN));

            // Mobility
            int mobility = Bitboards::popcount(att & safeSquares);
            add_eval<side, test>(activity, Interpolation::interpolate_gs(QueenMobility[mobility], stage), "Queen mobility");

            // King tropism
            Bitboard betweenArea = Board::aligned(sq, enemyKingPos) ? Board::Paths[sq][enemyKingPos] : Board::Boxes[sq][enemyKingPos];
            int blockers = Bitboards::popcount(betweenArea & ourPiecesExcluded);
            int tropism = (Pieces::pseudo_attacks(QUEEN, sq) & kingArea[enemy]) ? KingTropismQueenPoints[blockers] : KingTropismQueenPoints[blockers] >> 1;
            safetyPoints[enemy] += tropism;

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, att | xray, KingAreaAttackWages[QUEEN]);
        }

        // King attacks on king areas
        if (Pieces::piece_attacks_s<KING>(board->kingPosition(side)) & kingArea[enemy])
            kingAreaAttackers[enemy]++;

        return material + activity;
    }

    template Value Evaluator::evaluatePieces<WHITE>();
    template Value Evaluator::evaluatePieces<BLACK>();


    template <Color side>
    Value Evaluator::evaluateKing()
    {
        Value result = 0;
        constexpr bool test = false;

        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
        constexpr Direction forwardLeftDir = (side == WHITE ? NORTH_WEST : SOUTH_WEST);
        constexpr Bitboard shieldedArea1 = (side == WHITE ? 0x00ffffffffffffff : 0xffffffffffffff00);
        constexpr Bitboard shieldedArea2 = (side == WHITE ? 0x0000ffffffffffff : 0xffffffffffff0000);
        constexpr int shieldShift1 = DIRECTION_SHIFTS[forwardLeftDir];
        constexpr int shielfShift2 = DIRECTION_SHIFTS[forwardLeftDir] + DIRECTION_SHIFTS[forwardDir] - 3;
        Square kingSq = board->kingPosition(side);

        // King - pawns proximity (as a weighted average)
        Value proximityValue = Interpolation::interpolate_gs(parameters[KING_PAWN_PROXIMITY_VALUE], stage) * (proximityPoints[side] - proximityWages[side]) / proximityWages[side];
        add_eval<side, test>(result, proximityValue, "King and pawns proximity");

        // Pawn shield
        Bitboard ourPawns = board->pieces(side, PAWN);
        Bitboard firstShieldId = (shieldedArea1 & kingSq) ? (ourPawns & kingFrontSpans[side][0]) >> (kingSq + shieldShift1) : 0;
        Bitboard secondShield = (shieldedArea2 & kingSq) ? (ourPawns & kingFrontSpans[side][1]) >> (kingSq + shielfShift2) : 0;
        Bitboard shieldId = firstShieldId | secondShield;
        if (file_of(kingSq) == 0)
            shieldId |= 0x1;
        else if (file_of(kingSq) == 7)
            shieldId |= 0x4;
        safetyPoints[side] += PawnShieldPoints[shieldId];

        // King area attacks
        kingAreaAttackers[side] = std::max(kingAreaAttackers[side], 1);
        int attackPoints = kingAreaAttackPoints[side] * kingAreaAttackers[side] * kingAreaAttackers[side];
        safetyPoints[side] += attackPoints;

        return result;
    }

    template Value Evaluator::evaluateKing<WHITE>();
    template Value Evaluator::evaluateKing<BLACK>();

}