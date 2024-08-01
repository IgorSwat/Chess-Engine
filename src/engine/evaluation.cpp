#include "evaluation.h"
#include <algorithm>

namespace Evaluation {

    // ----------------------------
    // Helper functions and defines
    // ----------------------------

    constexpr Bitboard BoardHalves[COLOR_RANGE] = { 0x00000000ffffffff, 0xffffffff00000000 };
    constexpr Bitboard FianchettoMap[COLOR_RANGE] = { 0x0000000000244281, 0x8142240000000000 };

    
    // ---------------------
    // Main Evaluation class
    // ---------------- ----

    template <Color side>
    void Evaluator::initCommonData()
    {
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);

        stage = board->gameStage();

        // Piece imbalance
        pieceCount[side][ALL_PIECES] = Bitboards::popcount(board->pieces(side)) - 1;

        // Piece-attack properties
        Bitboard pawns = board->pieces(side, PAWN);
        attacks[side][PAWN] = Pieces::pawn_attacks<side>(pawns);
        attacks[side][KING] = Pieces::piece_attacks_s<KING>(board->kingPosition(side));
        attacks[side][ALL_PIECES] = attacks[side][PAWN] | attacks[side][KING];
        multipleAttacks[side][PAWN] = Pieces::pawn_attacks_x2<side>(pawns);

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
        weakKingSpan[side] = kingFrontSpans[side][1] & (~multipleAttacks[side][PAWN]);

        // Other common properties
        centralDensity = std::min(Bitboards::popcount(Board::CENTRAL_RANKS & board->pieces()), MAX_CENTRAL_DENSITY);
        boardDensity = Bitboards::popcount(board->pieces());
    }


    template <Color side>
    Value Evaluator::evaluatePawns1()
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

        weakPawns[side] = 0;
        passedPawns[side] = 0;

        pieceCount[side][PAWN] = Bitboards::popcount(ourPawns);
        add_eval<side, test>(result, Interpolation::interpolate_gs(PAWN_BASE_VALUE, stage) * pieceCount[side][PAWN], "Pawns material value");
        while (pawns) {
            Square sq = Bitboards::pop_lsb(pawns);
            Square front = sq + forwardDir;

            bool isDefendedByPawns = attacks[side][PAWN] & sq;
            bool isBlocked = board->pieces(PAWN) & front;
            int pawnsInFront = Bitboards::popcount(Board::FrontSpan[sq][side] & Board::Files[file_of(sq)] & ourPawns);

            // Pawn structure-wise strength
            if (isDefendedByPawns) {
                structurePoints[side][color_of(sq)] += isBlocked ? DEFENDED_BLOCKED_PAWN : GOOD_PAWN;
            }
            // Isolated pawn
            else if (!(Board::AdjacentFiles[sq] & ourPawns)) {
                structurePoints[side][color_of(sq)] += isBlocked ? ISOLATED_BLOCKED_PAWN : ISOLATED_MOBILE_PAWN;
                weakPawns[side] |= sq;
            }
            // Backward pawn
            else if (!(Board::back_span_m<side>(sq) & ourPawns) && ((enemyPawns | attacks[enemy][PAWN]) & front)) {
                structurePoints[side][color_of(sq)] += BACKWARD_PAWN;
                weakPawns[side] |= sq;
            }
            // Undefended, but neither isolated or backward
            else {
                structurePoints[side][color_of(sq)] += isBlocked ? UNDEFENDED_BLOCKED_PAWN : UNDEFENDED_MOBILE_PAWN;
            }

            // Doubled pawn
            if (pawnsInFront > 0) {
                Value penalty = std::max(Interpolation::interpolate_gs(DOUBLED_PAWN_PENALTY, stage) * pawnsInFront * pawnsInFront, DOUBLED_PAWN_MAX_PENALTY);
                add_eval<side, test>(result, penalty, "Doubled pawn penalty");

                if (!isDefendedByPawns)
                    updateProximity<side>(sq, OUR_PAWN_PROXIMITY_FACTOR, ENEMY_PAWN_PROXIMITY_FACTOR);
            }
            // Passed pawn
            else if (!(Board::FrontSpan[sq][side] & enemyPawns)) {
                int promotionDistance = (side == WHITE ? 7 - rank_of(sq) : rank_of(sq));

                updateProximity<side>(sq, OUR_PASSER_PROXIMITY_FACTOR, ENEMY_PASSER_PROXIMITY_FACTOR);
                if (!stage && Board::SquareDistance[board->kingPosition(enemy)][make_square(promotionRank, file_of(sq))] - (board->movingSide() == enemy) > promotionDistance)
                    mostAdvancedUnstopablePasser = std::min(promotionDistance, mostAdvancedUnstopablePasser);
                else
                    passedPawns[side] |= sq;
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
            Value passerValue = ((8 - mostAdvancedUnstopablePasser) * QUEEN_BASE_VALUE.opening) >> 3;
            add_eval<side, test>(result, passerValue, "Unstopable passed pawn");
        }

        return result;
    }


    template <Color side>
    Value Evaluator::evaluatePawns2()
    {
        Value result = 0;
        constexpr bool test = false;

        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
        Bitboard weak = weakPawns[side] & attacks[enemy][ALL_PIECES];
        Bitboard passed = passedPawns[side];

        // Weak pawns
        while (weak) {
            Bitboard sqBB = square_to_bb(Bitboards::pop_lsb(weak));
            Value penalty = Interpolation::interpolate_gs(WEAK_PAWN_ATTACKED, stage) * countAttackers(sqBB, enemy, KNIGHT, BISHOP, ROOK, QUEEN, KING);
            add_eval<side, test>(result, penalty, "Weak pawn attacked");
        }

        // Passed pawns
        int points = 0;
        Bitboard processedPassers = 0;
        while (passed) {
            Square sq = Bitboards::pop_lsb(passed);
            int promotionDistance = (side == WHITE ? 7 - rank_of(sq) : rank_of(sq));
            Bitboard promotionPath = Board::FrontSpan[sq][side] & Board::Files[file_of(sq)];
            Bitboard behindPawn = Board::back_span<side>(sq) & Pieces::piece_attacks_s<ROOK>(sq, board->pieces());
            Bitboard connectedPassers = Board::AdjacentFiles[sq] & processedPassers;

            // Protected passer
            if (!(Board::AdjacentFiles[sq] & passedPawns[side]) && (attacks[side][PAWN] & sq))
                promotionDistance--;

            // Promotion path control
            int control = -countAttackers(promotionPath, enemy, KNIGHT, BISHOP, ROOK, QUEEN, KING);
            control -= Bitboards::popcount(promotionPath & board->pieces(enemy, KNIGHT, BISHOP));
            if (promotionPath & attacks[side][ALL_PIECES])
                control += countAttackers(promotionPath, side, KNIGHT, BISHOP, ROOK, QUEEN, KING);
            if (board->pieces(enemy) & square_to_bb(sq + forwardDir))
                control += BLOCKED_PASSER_PENALTY_SIZE;


            // Heavy pieces behind passed pawns
            if (board->pieces(side, ROOK, QUEEN) & behindPawn) {
                control++;
                passerPoints[sq] = PASSED_PAWN_POINTS[promotionDistance][16 + control] + HEAVY_PIECE_BEHIND_PASSER[promotionDistance];
            }
            else if (board->pieces(enemy, ROOK, QUEEN) & behindPawn) {
                control--;
                passerPoints[sq] = PASSED_PAWN_POINTS[promotionDistance][16 + control] - HEAVY_PIECE_BEHIND_PASSER[promotionDistance];
            }
            else
                passerPoints[sq] = PASSED_PAWN_POINTS[promotionDistance][16 + control];

            // Connected passer
            while (connectedPassers) {
                Square supporterSq = Bitboards::pop_lsb(connectedPassers);
                points += (passerPoints[sq] * passerPoints[supporterSq] * CONNECTED_PASSERS_FACTOR) >> 10;
            }

            // Mark as processed
            processedPassers = passedPawns[side] ^ passed;
            points += passerPoints[sq];
        }

        // Point to eval scaling
        if (points)
            add_eval<side, test>(result, (points * Interpolation::interpolate_gs(PASSED_PAWNS_VALUE, stage)) >> 6, "Passed pawns");

        return result;
    }


    template <Color side>
    Value Evaluator::evaluatePieces()
    {
        Value result = 0;
        constexpr bool test = false;

        constexpr Color enemy = ~side;
        constexpr Bitboard rank_78 = (side == WHITE ? Board::RANK_7 | Board::RANK_8 : Board::RANK_1 | Board::RANK_2);
        Bitboard ourPawns = board->pieces(side, PAWN);
        Bitboard ourPieces = board->pieces(side) ^ ourPawns;
        Bitboard enemyPawns = board->pieces(enemy, PAWN);
        Bitboard enemyPieces = board->pieces(enemy) ^ enemyPawns;
        Bitboard ourPiecesExcluded = board->pieces() ^ ourPieces;
        Bitboard safeSquares = (Board::BOARD ^ (attacks[enemy][PAWN] | board->pieces(side))) | enemyPieces;     // TODO: fix this
        Square enemyKingPos = board->kingPosition(enemy);

        // Knights
        Bitboard knights = board->pieces(side, KNIGHT);

        pieceCount[side][KNIGHT] = Bitboards::popcount(knights);
        attacks[side][KNIGHT] = multipleAttacks[side][KNIGHT] = 0;

        add_eval<side, test>(result, Interpolation::interpolate_gs(KNIGHT_BASE_VALUE, stage) * pieceCount[side][KNIGHT], "Knights material value");
        add_eval<side, test>(result, KNIGHT_POSITION_DENSITY[centralDensity] * pieceCount[side][KNIGHT], "Knights board density value");
        add_eval<side, test>(result, Interpolation::interpolate_gs(KNIGHT_PAWN_SPREAD[pawnRankSpread], stage) * pieceCount[side][KNIGHT], "Knights pawn spread compensation");
        while (knights) {
            Square sq = Bitboards::pop_lsb(knights);
            Bitboard att = Pieces::piece_attacks_s<KNIGHT>(sq);

            multipleAttacks[side][KNIGHT] |= attacks[side][KNIGHT] & att;
            attacks[side][KNIGHT] |= att;
            attacks[side][ALL_PIECES] |= att;

            int mobility = Bitboards::popcount(att & safeSquares);
            add_eval<side, test>(result, Interpolation::interpolate_gs(KNIGHT_MOBILITY[mobility], stage), "Knight mobility");

            // Outposts
            if (attacks[side][PAWN] & safeSquares) {
                // Possibility to attack outpost with pawn, II deg outpost
                if (enemyPawns & Board::FrontSpan[sq][side] & Board::AdjacentFiles[sq])
                    add_eval<side, test>(result, (Interpolation::interpolate_gs(KNIGHT_OUTPOST_II_DEG, stage) * OutpostFactors[side][sq]) >> 4, "Knight II deg outpost");
                // Uncontested, III deg outpost
                else
                    add_eval<side, test>(result, (Interpolation::interpolate_gs(KNIGHT_OUTPOST_III_DEG, stage) * OutpostFactors[side][sq]) >> 4, "Knight III deg outpost");
            }
            else if (attacks[side][ALL_PIECES] & safeSquares)
                add_eval<side, test>(result, (Interpolation::interpolate_gs(KNIGHT_OUTPOST_I_DEG, stage) * OutpostFactors[side][sq]) >> 4, "Knight I deg outpost");

            // King tropism
            int tropismDistance = (Board::SquareDistance[sq][enemyKingPos] - 1) >> 1;
            int tropism = KingTropismKnightPoints >> tropismDistance;
            safetyPoints[enemy] += tropism;

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, att, KingAreaAttackWages[KNIGHT]);
        }

        // Bishops
        Bitboard bishops = board->pieces(side, BISHOP);

        pieceCount[side][BISHOP] = Bitboards::popcount(bishops);
        attacks[side][BISHOP] = multipleAttacks[side][BISHOP] = 0;

        add_eval<side, test>(result, Interpolation::interpolate_gs(BISHOP_BASE_VALUE, stage) * pieceCount[side][BISHOP], "Bishops material value");
        if ((bishops & Board::LIGHT_SQUARES) && (bishops & Board::DARK_SQUARES))
            add_eval<side, test>(result, Interpolation::interpolate_gs(BISHOP_PAIR_VALUE, stage), "Bishop pair bonus");
        while (bishops) {
            Square sq = Bitboards::pop_lsb(bishops);
            SquareColor color = color_of(sq);
            Bitboard colorMap = (color == LIGHT_SQUARE ? Board::LIGHT_SQUARES : Board::DARK_SQUARES) & Board::NOT_EDGE_FILES;
            Bitboard att = Pieces::piece_attacks_s<BISHOP>(sq, board->pieces());
            Bitboard attAndXray = att | Pieces::xray_attacks<BISHOP>(sq, board->pieces(), board->pieces(side, BISHOP, QUEEN));

            multipleAttacks[side][BISHOP] |= attacks[side][BISHOP] & attAndXray;
            attacks[side][BISHOP] |= att;
            attacks[side][ALL_PIECES] |= att;

            // Our pawn blockage
            int ourBlockage = std::min(Bitboards::popcount(board->pieces(side, PAWN) & colorMap), 6);
            add_eval<side, test>(result, Interpolation::interpolate_gs(BISHOP_OWN_PAWN_BLOCKAGE[ourBlockage], stage), "Own pawn placement (bishop)");

            // Enemy pawn blockage
            int enemyBlockage = std::min(Bitboards::popcount(enemyPawns & colorMap), 6);
            add_eval<side, test>(result, Interpolation::interpolate_gs(BISHOP_ENEMY_PAWN_BLOCKAGE[enemyBlockage], stage), "Enemy pawn placement (bishop)");

            // Enemy structure weakness
            Value structureWeaknessEval = (Interpolation::interpolate_gs(BISHOP_ENEMY_PAWN_WEAKNESS, stage) * structurePoints[enemy][color]) >> 4;
            add_eval<side, test>(result, structureWeaknessEval, "Enemy pawn structure weakness (bishop)");

            // Mobility and fianchetto
            Bitboard occ = (FianchettoMap[side] & sq) ? board->pieces() ^ ourPieces : board->pieces();
            int mobility = Bitboards::popcount(Pieces::piece_attacks_s<BISHOP>(sq, occ) & safeSquares);
            add_eval<side, test>(result, Interpolation::interpolate_gs(BISHOP_MOBILITY[mobility], stage), "Bishop mobility");

            // Outposts
            if (attacks[side][PAWN] & sq) {
                // Possibility to attack outpost with pawn, II deg outpost
                if (enemyPawns & Board::FrontSpan[sq][side] & Board::AdjacentFiles[sq])
                    add_eval<side, test>(result, (Interpolation::interpolate_gs(BISHOP_OUTPOST_II_DEG, stage) * OutpostFactors[side][sq]) >> 4, "Bishop II deg outpost");
                // Uncontested, III deg outpost
                else
                    add_eval<side, test>(result, (Interpolation::interpolate_gs(BISHOP_OUTPOST_III_DEG, stage) * OutpostFactors[side][sq]) >> 4, "Bishop III deg outpost");
            }
            else if (attacks[side][ALL_PIECES] & sq)
                add_eval<side, test>(result, (Interpolation::interpolate_gs(BISHOP_OUTPOST_I_DEG, stage) * OutpostFactors[side][sq]) >> 4, "Bishop I deg outpost");

            // King tropism
            int blockers = Bitboards::popcount(Board::Boxes[sq][enemyKingPos] & ourPiecesExcluded);
            int tropism = (Pieces::pseudo_attacks(BISHOP, sq) & kingArea[enemy]) ? KingTropismBishopPoints[blockers] : KingTropismBishopPoints[blockers] >> 1;
            safetyPoints[enemy] += tropism;

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, attAndXray, KingAreaAttackWages[BISHOP]);
        }

        // Rooks
        Bitboard rooks = board->pieces(side, ROOK);
        int structureWeakness = structurePoints[enemy][LIGHT_SQUARE] + structurePoints[enemy][DARK_SQUARE];

        pieceCount[side][ROOK] = Bitboards::popcount(rooks);
        attacks[side][ROOK] = multipleAttacks[side][ROOK] = 0;

        add_eval<side, test>(result, Interpolation::interpolate_gs(ROOK_BASE_VALUE, stage) * pieceCount[side][ROOK], "Rooks material value");
        add_eval<side, test>(result, (Interpolation::interpolate_gs(ROOK_ENEMY_PAWN_WEAKNESS, stage) * pieceCount[side][ROOK] * structureWeakness) >> 5, "Enemy pawn structure weakness (rook)");
        while (rooks) {
            Square sq = Bitboards::pop_lsb(rooks);
            Bitboard fileBB = Board::Files[file_of(sq)];
            Bitboard att = Pieces::piece_attacks_s<ROOK>(sq, board->pieces());
            Bitboard attAndXray = att | Pieces::xray_attacks<ROOK>(sq, board->pieces(), board->pieces(side, ROOK, QUEEN));

            multipleAttacks[side][ROOK] |= attacks[side][ROOK] & attAndXray;
            attacks[side][ROOK] |= att;
            attacks[side][ALL_PIECES] |= att;

            // Placement bonuses
            if (rank_78 & sq)
                add_eval<side, test>(result, Interpolation::interpolate_gs(ROOK_ON_78_RANK, stage), "Rook on 7-8 rank");
            else if (!(fileBB & board->pieces(PAWN)))
                add_eval<side, test>(result, Interpolation::interpolate_gs(ROOK_ON_OPEN_FILE, stage), "Rook on open file");
            else if (!(fileBB & ourPawns))
                add_eval<side, test>(result, Interpolation::interpolate_gs(ROOK_ON_SEMIOPEN_FILE, stage), "Rook on semiopen file");

            // Mobility
            int mobility = Bitboards::popcount(att & safeSquares);
            add_eval<side, test>(result, Interpolation::interpolate_gs(ROOK_MOBILITY[mobility], stage), "Rook mobility");

            // King tropism
            int blockers = Bitboards::popcount(Board::Boxes[sq][enemyKingPos] & ourPiecesExcluded);
            int tropism = (Pieces::pseudo_attacks(ROOK, sq) & kingArea[enemy]) ? KingTropismRookPoints[blockers] : KingTropismRookPoints[blockers] >> 1;
            safetyPoints[enemy] += tropism;

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, attAndXray, KingAreaAttackWages[ROOK]);
        }

        // Queens
        Bitboard queens = board->pieces(side, QUEEN);

        pieceCount[side][QUEEN] = Bitboards::popcount(queens);
        attacks[side][QUEEN] = multipleAttacks[side][QUEEN] = 0;

        add_eval<side, test>(result, Interpolation::interpolate_gs(QUEEN_BASE_VALUE, stage) * pieceCount[side][QUEEN], "Queens material value");
        add_eval<side, test>(result, (Interpolation::interpolate_gs(QUEEN_ENEMY_PAWN_WEAKNESS, stage) * pieceCount[side][QUEEN] * structureWeakness) >> 5, "Enemy pawn structure weakness (queen)");
        while (queens) {
            Square sq = Bitboards::pop_lsb(queens);
            Bitboard att = Pieces::piece_attacks_s<QUEEN>(sq, board->pieces());
            Bitboard attAndXray = att | Pieces::xray_attacks<QUEEN>(sq, board->pieces(), board->pieces(side, BISHOP, ROOK, QUEEN));

            multipleAttacks[side][QUEEN] |= attacks[side][QUEEN] & attAndXray;
            attacks[side][QUEEN] |= att;
            attacks[side][ALL_PIECES] |= att;

            // Mobility
            int mobility = Bitboards::popcount(att & safeSquares);
            add_eval<side, test>(result, Interpolation::interpolate_gs(QUEEN_MOBILITY[mobility], stage), "Queen mobility");

            // King tropism
            Bitboard betweenArea = Board::aligned(sq, enemyKingPos) ? Board::Paths[sq][enemyKingPos] : Board::Boxes[sq][enemyKingPos];
            int blockers = Bitboards::popcount(betweenArea & ourPiecesExcluded);
            int tropism = (Pieces::pseudo_attacks(QUEEN, sq) & kingArea[enemy]) ? KingTropismQueenPoints[blockers] : KingTropismQueenPoints[blockers] >> 1;
            safetyPoints[enemy] += tropism;

            // King area attacks & defence
            updateKingAreaSafety<side>(sq, attAndXray, KingAreaAttackWages[QUEEN]);
        }

        // King attacks on king areas
        if (Pieces::piece_attacks_s<KING>(board->kingPosition(side)) & kingArea[enemy])
            kingAreaAttackers[enemy]++;

        return result;
    }


    template <Color side>
    Value Evaluator::evaluateKingAndMisc()
    {
        Value result = 0;
        constexpr bool kingTest = false;
        constexpr bool miscTest = false;

        constexpr Color enemy = ~side;
        constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
        constexpr Direction backwardDir = ~forwardDir;
        constexpr Direction forwardLeftDir = (side == WHITE ? NORTH_WEST : SOUTH_WEST);
        constexpr Bitboard shieldedArea1 = (side == WHITE ? 0x00ffffffffffffff : 0xffffffffffffff00);
        constexpr Bitboard shieldedArea2 = (side == WHITE ? 0x0000ffffffffffff : 0xffffffffffff0000);
        constexpr Bitboard rank1 = (side == WHITE ? Board::RANK_1 : Board::RANK_8);
        constexpr Bitboard ranks234 = (side == WHITE ? Board::RANK_2 | Board::RANK_3 | Board::RANK_4 : Board::RANK_7 | Board::RANK_6 | Board::RANK_5);
        constexpr Bitboard center = Board::CENTRAL_FILES;
        constexpr Bitboard outside = Board::BOARD ^ center;
        constexpr int shieldShift1 = DIRECTION_SHIFTS[forwardLeftDir];
        constexpr int shieldShift2 = DIRECTION_SHIFTS[forwardLeftDir] + DIRECTION_SHIFTS[forwardDir] - 3;
        Square kingSq = board->kingPosition(side);

        // King - pawns proximity (as a weighted average)
        Value proximityValue = proximityWages[side] ? Interpolation::interpolate_gs(KING_PAWN_PROXIMITY_VALUE, stage) * (proximityPoints[side] - proximityWages[side]) / proximityWages[side] : 0;
        add_eval<side, kingTest>(result, proximityValue, "King and pawns proximity");

        // Pawn shield
        Bitboard ourPawns = board->pieces(side, PAWN);
        Bitboard firstShieldId = (shieldedArea1 & kingSq) ? (ourPawns & kingFrontSpans[side][0]) >> (kingSq + shieldShift1) : 0;
        Bitboard secondShield = (shieldedArea2 & kingSq) ? (ourPawns & kingFrontSpans[side][1]) >> (kingSq + shieldShift2) : 0;
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

        // Global king safety
        add_eval<side, kingTest>(result, (safetyPoints[side] * Interpolation::interpolate_gs(KING_SAFETY_VALUE, stage)) >> 6, "King safety");


        // Other evaluation features (compressed here for efficiency reasons)

        // Space
        Bitboard space = (Bitboards::fill<backwardDir>(ourPawns) ^ ourPawns);
        Bitboard effectiveSpace = space & ranks234;
        Bitboard uncontestedPawnSpace = effectiveSpace & ~attacks[enemy][ALL_PIECES];
        Bitboard contestedPawnSpace = effectiveSpace & (attacks[enemy][ALL_PIECES] ^ attacks[enemy][PAWN]);
        Bitboard pawnlessSpace = Bitboards::fill<forwardDir>(rank1 ^ (space & rank1)) & ranks234 & attacks[side][ALL_PIECES] & ~attacks[enemy][ALL_PIECES];

        int spaceTypeI = Bitboards::popcount(contestedPawnSpace & center) * CENTRAL_SPACE_POINTS + Bitboards::popcount(contestedPawnSpace & outside) * OTHER_SPACE_POINTS +
                         Bitboards::popcount(pawnlessSpace & center) * CENTRAL_SPACE_POINTS + Bitboards::popcount(pawnlessSpace & outside) * OTHER_SPACE_POINTS;
        int spaceTypeII = Bitboards::popcount(uncontestedPawnSpace & center) * CENTRAL_SPACE_POINTS + Bitboards::popcount(uncontestedPawnSpace & outside) * OTHER_SPACE_POINTS;
        add_eval<side, miscTest>(result, (spaceTypeI * Interpolation::interpolate_gs(SPACE_TYPE_I, stage)) >> 6, "Space type I");
        add_eval<side, miscTest>(result, (spaceTypeII * Interpolation::interpolate_gs(SPACE_TYPE_II, stage)) >> 6, "Space type II");

        // Lower type attack threats
        Bitboard safePieces = board->pieces(side, KNIGHT, BISHOP, ROOK, QUEEN);
        Bitboard pawnThreats = safePieces & attacks[enemy][PAWN];
        int pawnThreatCount = Bitboards::popcount(pawnThreats);
        safePieces ^= pawnThreats;
        Bitboard minorPieceThreats = safePieces & board->pieces(side, ROOK, QUEEN) & (attacks[enemy][KNIGHT] | attacks[enemy][BISHOP]);
        int minorPieceThreatCount = Bitboards::popcount(minorPieceThreats);
        safePieces ^= minorPieceThreats;
        Bitboard rookThreats = safePieces & board->pieces(side, QUEEN) & attacks[enemy][ROOK];
        int rookThreatCount = Bitboards::popcount(rookThreats);
        safePieces ^= rookThreats;

        // Undefended piece threats
        Bitboard undefendedAndAttacked = attacks[enemy][ALL_PIECES] & ~attacks[side][ALL_PIECES];
        int undefendedPieceThreatCount = Bitboards::popcount(safePieces & undefendedAndAttacked);

        // Calculate all piece threats together
        threatCount[side] = pawnThreatCount + minorPieceThreatCount + rookThreatCount + undefendedPieceThreatCount;
        int threatPoints = pawnThreatCount * PAWN_EXCHANGE_THREAT_POINTS + minorPieceThreatCount * MINOR_PIECE_EXCHANGE_THREAT_POINTS +
                           rookThreatCount * ROOK_EXCHANGE_THREAT_POINTS + undefendedPieceThreatCount * UNDEFENDED_PIECE_THREAT_POINTS;

        // Undefended pawn threats (requires some tests for it's utility)
        if constexpr (EVALUATE_HNGING_PAWN_THREATS) {
            int undefendedPawnThreatCount = Bitboards::popcount(ourPawns & undefendedAndAttacked);
            threatCount[side] += undefendedPawnThreatCount;
            threatPoints += undefendedPawnThreatCount * UNDEFENDED_PAWN_THEAT_POINTS;
        }

        // Mate threats
        // ???

        add_eval<side, miscTest>(result, (threatCount[side] * threatPoints * Interpolation::interpolate_gs(THREAT_VALUE, stage)) >> 6, "Threats");


        return result;
    }


    Value Evaluator::adjustEval(Value eval) const
    {
        constexpr bool test = false;

        int noPawns = pieceCount[WHITE][PAWN] + pieceCount[BLACK][PAWN];
        int noPassedPawns = Bitboards::popcount(passedPawns[WHITE] | passedPawns[BLACK]);
        int noPieces = pieceCount[WHITE][ALL_PIECES] + pieceCount[BLACK][ALL_PIECES] - noPawns;
        int pieceImbalance = std::abs(pieceCount[WHITE][KNIGHT] - pieceCount[BLACK][KNIGHT]) * PieceImbalancePoints[KNIGHT] +
                             std::abs(pieceCount[WHITE][BISHOP] - pieceCount[BLACK][BISHOP]) * PieceImbalancePoints[BISHOP] +
                             std::abs(pieceCount[WHITE][ROOK] - pieceCount[BLACK][ROOK]) * PieceImbalancePoints[ROOK] +
                             std::abs(pieceCount[WHITE][QUEEN] - pieceCount[BLACK][QUEEN]) * PieceImbalancePoints[QUEEN];

        bool pawnsOnBothSides = (Board::QUEENSIDE & board->pieces(PAWN)) &&
                                (Board::KINGSIDE & board->pieces(PAWN));

        bool kingInfiltration = (BoardHalves[BLACK] & board->kingPosition(WHITE)) ||
                                (BoardHalves[WHITE] & board->kingPosition(BLACK));

        int threshold = COMPLEXITY_THRESHOLD;


        // Special endgames
        // Opposite color bishops endgame
        if (stage == SINGLE_MINOR_VS_MINOR_ENDGAME && detectOppositeColorBishops()) {
            // In the case of such endgame we decrease the 
            Value adjustment = (pieceCount[WHITE][PAWN] - pieceCount[BLACK][PAWN]) * OPPOSITE_COLOR_BISHOPS_PAWN_ADJUSTMENT;
            eval -= adjustment;
        }
        // Drawish rook endgame
        else if (stage == SINGLE_ROOK_VS_ROOK_ENDGAME && !pawnsOnBothSides && !noPassedPawns) {
            Value adjustment = (pieceCount[WHITE][PAWN] - pieceCount[BLACK][PAWN]) * DRAWISH_ROOK_ENDGAME_ADJUSTMENT;
            eval -= adjustment;
        }
        // Up a piece endgames
        else if (stage == SINGLE_MINOR_ENDGAME) {
            Color sideWithPiece = board->pieces(WHITE, KNIGHT, BISHOP) ? WHITE : BLACK;
            // If winning side has pawns, we increase the eval
            if (sideWithPiece == WHITE && !pieceCount[WHITE][PAWN])
                eval = std::min(eval, 0);
            else if (sideWithPiece == WHITE)
                eval += UP_A_PIECE_ENDGAME_ADJUSTMENT;
            else if (sideWithPiece == BLACK && !pieceCount[BLACK][PAWN])
                eval = std::max(eval, 0);
            else
                eval -= UP_A_PIECE_ENDGAME_ADJUSTMENT;
        }
        else if (stage == SINGLE_ROOK_ENDGAME) {
            eval += board->pieces(WHITE, ROOK) ? UP_A_PIECE_ENDGAME_ADJUSTMENT : -UP_A_PIECE_ENDGAME_ADJUSTMENT;
        }
        // Adjust threshold when no pieces are on the board
        else if (stage == PAWN_ENDGAME) {
            threshold = PAWN_ENDGAME_COMPLEXITY_THRESHOLD;
        }


        int points = noPawns * PAWN_COMPLEXITY_POINTS +
                     noPassedPawns * PASSED_PAWN_COMPLEXITY_POINTS +
                     noPieces * PIECE_COMPLEXITY_POINTS +
                     kingInfiltration * INFILTRATION_COMPLEXITY_POINTS +
                     pawnsOnBothSides * PAWNS_ON_BOTH_SIDES_COMPLEXITY_POINTS +
                     pieceImbalance;

        if constexpr (test)
            std::cout << "Winning chances adjustment: " << points << "/" << threshold << std::endl;

        Value finalScore = eval * std::min(points, threshold) / threshold;

        // Shuffling (no progress) adjustment
        finalScore = finalScore * (100 - board->halfmovesClocked()) / 100;

        return finalScore;
    }


    Value Evaluator::evaluate()
    {
        initCommonData<WHITE>();
        initCommonData<BLACK>();

        Value result = evaluatePawns1<WHITE>() - evaluatePawns1<BLACK>() +
                       evaluatePieces<WHITE>() - evaluatePieces<BLACK>() +
                       evaluatePawns2<WHITE>() - evaluatePawns2<BLACK>() +
                       evaluateKingAndMisc<WHITE>() - evaluateKingAndMisc<BLACK>();

        return adjustEval(result);
    }

}