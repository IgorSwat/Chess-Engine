#include "testEngine.h"
#include "../engine/searchConfig.h"

using namespace Search;


// --------------------------
// TestEngine methods - setup
// --------------------------

void TestEngine::setPosition(BoardConfig *board)
{
    virtualBoard.loadPosition(*board);
    rootAge = virtualBoard.halfmovesPlain();
}

void TestEngine::setPosition(const std::string &fen)
{
    virtualBoard.loadPosition(fen);
    rootAge = virtualBoard.halfmovesPlain();
}


// -------------------------------
// TestEngine methods - main usage
// -------------------------------

constexpr Value MAX_EVAL = std::numeric_limits<Value>::max();   // Used as a checkmate evaluation and upper boundary for beta

Value TestEngine::evaluate_s()
{
    return relative_score(evaluate(), &virtualBoard);
}

Value TestEngine::evaluate_d(Depth depth)
{
    lastUsedDepth = depth;

    std::cout << "Position hash: " << virtualBoard.hash() << ", positon pieces: " << virtualBoard.pieces() << "\n";
    // Show queiescence procedure in details
    if (depth == 0) {
        std::cout << "|||||   Queiescence logs   |||||\n";
        SearchResult result = quiescence<Q_ROOT_STAGE, true>(-MAX_EVAL, MAX_EVAL, MAX_QUIESCENCE_DEPTH);
        std::cout << "\n|||||   Queiescence results   |||||\n";
        std::cout << std::dec << "Score: " << relative_score(result.score, &virtualBoard);
        std::cout << ", Type: " << int(result.nodeType) << ", Best (queiescence) move: " << result.bestMove << "\n";
        return relative_score(result.score, &virtualBoard);
    }
    else {
        SearchResult result = search<ROOT_STAGE_I>(-MAX_EVAL, MAX_EVAL, depth);
        std::cout << "\n|||||   Search results   |||||\n";
        std::cout << std::dec << "Score: " << relative_score(result.score, &virtualBoard);
        std::cout << ", Type: " << int(result.nodeType) << ", Best move: " << result.bestMove << "\n";
        return relative_score(result.score, &virtualBoard);
    }
}


// --------------------------------
// TestEngine methods - main search
// --------------------------------

template <SearchStage stage>
SearchResult TestEngine::search(Value alpha, Value beta, Depth depth)
{
    // 50-move rule
    if (virtualBoard.halfmovesClocked() >= 100) {
        std::cout << "DRAW by 50 move rules\n";
        return {0, TERMINAL_NODE, Move::null()};
    }

    // Repetitions
    if (virtualBoard.irreversibleMoveDistance() >= 4) {
        std::uint16_t repetitions = virtualBoard.countRepetitions();

        if (repetitions == 3) { // A definite 3-fold 
            std::cout << "DRAW by 3-fold repetition\n";
            return {0, TERMINAL_NODE, Move::null()};
        }
        if (repetitions == 2 && lastUsedDepth - depth > virtualBoard.irreversibleMoveDistance()) { // A repetition inside search space
            std::cout << "2-fold inside current search tree\n";
            return {0, TERMINAL_NODE, Move::null()};
        }
    }

    // Check the transposition table
    const TranspositionTable::Entry *entry = tTable.probe(virtualBoard.hash(), virtualBoard.pieces());
    Move suggestedMove;
    if (entry) {
        if constexpr (stage == ROOT_STAGE_I) {
            std::cout << "[TestEngine]: found existing entry in T.Table: [score: " << relative_score(entry->score, &virtualBoard);
            std::cout << ", type: " << int(entry->typeOfNode) << ", best " << entry->bestMove << "]\n";
        }
        // if (entry->depth >= depth || entry->typeOfNode == TERMINAL_NODE)
        //     return {entry->score, entry->typeOfNode, entry->bestMove};
        // suggestedMove = entry->bestMove;
        // Note: in contrast to normal engine, we do not return result immediately 
    }

    if (depth == 0)
        return quiescence<Q_ROOT_STAGE, false>(alpha, beta, MAX_QUIESCENCE_DEPTH);

    Value bestScore = -MAX_EVAL;
    NodeType nodeType = ALL_NODE;
    Move bestMove; // A null move by default

    // Initial move generation
    MoveSelector moveSelector(&virtualBoard);
    if constexpr (stage == ROOT_STAGE_I || stage == ROOT_STAGE_II) {
        moveSelector.generateMoves<MoveGeneration::LEGAL>();

        // Sort moves by static evaluation of the following position
        MoveSelection::sort_moves(moveSelector, [this](const Move &move) -> int64_t {
                this->virtualBoard.makeMove(move);
                Value eval = -this->evaluate();
                this->virtualBoard.undoLastMove();

                // This makes SEE a primary sorting parameter
                // Static evaluation is considered only when SEE of two moves is equal
                return 3LL * MAX_EVAL * SEE::evaluate(&this->virtualBoard, move) + eval;
        });
    }
    else {
        if (virtualBoard.isInCheck(virtualBoard.movingSide()))
            moveSelector.generateMoves<MoveGeneration::CHECK_EVASION>();
        else
            moveSelector.generateMoves<MoveGeneration::CAPTURE>();
    }

    // Detect mate, stealmate and other types of draw
    if (!moveSelector.hasMoves()) {
        Value score = virtualBoard.isInCheck() ? -MAX_EVAL : 0; // Mate or stealmate
        tTable.set({
            virtualBoard.hash(),
            virtualBoard.pieces(),
            depth,
            score,
            Move::null(),
            TERMINAL_NODE,
            virtualBoard.halfmovesPlain()
        }, rootAge);

        if constexpr (stage == ROOT_STAGE_I)
            std::cout << "Current position is mate / stealmate\n";

        return {score, TERMINAL_NODE, Move::null()};
    }

    // Specify selection strategy according to search stage
    constexpr GenerationStrategy genStrategy = stage == COMMON_STAGE ? GenerationStrategy::CASCADE : GenerationStrategy::STRICT;
    constexpr SelectionStrategy selStrategy = stage == COMMON_STAGE ? SelectionStrategy::STANDARD_ORDERING : SelectionStrategy::SIMPLE;
    constexpr SearchStage nextStage = stage == ROOT_STAGE_I ? ROOT_STAGE_II : COMMON_STAGE;

    if constexpr (stage == ROOT_STAGE_I)
        std::cout << "Analyzing the following moves:\n";

    // Main loop
    Move move = suggestedMove != Move::null() ? suggestedMove : moveSelector.selectNext<genStrategy, selStrategy>();
    while (move != Move::null()) {
        // Make move and evaluate further
        virtualBoard.makeMove(move);
        SearchResult result = search<nextStage>(-beta, -alpha, depth - 1);
        result.score = -result.score;
        virtualBoard.undoLastMove();

        if constexpr (stage == ROOT_STAGE_I) {
            std::cout << move << std::dec << " -> [score: " << relative_score(result.score, &virtualBoard);
            std::cout << ", type: " << int(result.nodeType) << ", best " << result.bestMove << "]\n";
        }

        // Beta cut-off: an enemy would never allow a line worse than beta to occur
        if (result.score >= beta) {
            tTable.set({
                virtualBoard.hash(),
                virtualBoard.pieces(),
                depth,
                result.score,
                move,
                CUT_NODE,
                virtualBoard.halfmovesPlain()
            }, rootAge);

            return {result.score, CUT_NODE, move};
        }

        // Note: this part is quite a bit different than the one from normal engine
        if (result.score > bestScore) {
            bestMove = move;
            bestScore = result.score;
            if (result.score > alpha) {
                alpha = result.score;
                nodeType = PV_NODE;
            }
        }

        move = moveSelector.selectNext<genStrategy, selStrategy>();

        // Experimental - avoid repeating of transposition table suggestion
        if (move != Move::null() && move == suggestedMove)
            move = moveSelector.selectNext<genStrategy, selStrategy>();
    }

    // Save search results in transposition table
    tTable.set({
        virtualBoard.hash(),
        virtualBoard.pieces(),
        depth,
        bestScore,
        bestMove,
        nodeType,
        virtualBoard.halfmovesPlain()
    }, rootAge);

    return {bestScore, nodeType, bestMove};
}


// ---------------------------------------
// TestEngine methods - queiescence search
// ---------------------------------------

template <SearchStage stage, bool trace>
SearchResult TestEngine::quiescence(Value alpha, Value beta, Depth depth)
{
    MoveSelector moveSelector(&virtualBoard);
    if (virtualBoard.isInCheck()) {
        moveSelector.generateMoves<MoveGeneration::CHECK_EVASION>();
        if constexpr (trace)
            std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: generating check evasions\n";
    }
    else {
        moveSelector.generateMoves<MoveGeneration::CAPTURE>();
        if constexpr (trace)
            std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: generating captures\n";
    }

    // Detect mate, stealmate and other types of draw
    if (!moveSelector.hasMoves()) {
        Value score = virtualBoard.isInCheck() ? -MAX_EVAL : 0;      // Mate or stealmate
        tTable.set({
            virtualBoard.hash(),
            virtualBoard.pieces(),
            depth,
            score,
            Move::null(),
            TERMINAL_NODE,
            virtualBoard.halfmovesPlain()
        }, rootAge);

        if constexpr (trace)
            std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: current position is mate / stealmate\n";

        // The score is exact, so we can finish searching the branch here
        return {score, TERMINAL_NODE, Move::null()};
    }

    // Stand pat score - a static evaluation to help asses whether some moves are worth trying out or not
    Value standpat = evaluate();

    // Maximum queiescence depth reached
    if (depth == 0) {
        if constexpr (trace)
            std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: max depth reached\n";
        return {standpat, PV_NODE, Move::null()};
    }

    Value bestScore = -MAX_EVAL;
    NodeType nodeType = ALL_NODE;
    Move bestMove;
    Move move;

    // Check all "good" captures which could boost the score near the alpha (upper bound) region
    if (moveSelector.currGenType >= MoveGeneration::CAPTURE) {
        // Experimental - sort moves by SEE at initial quiescence depth
        if constexpr (stage == Q_ROOT_STAGE)
            moveSelector.sortCaptures();

        constexpr SelectionStrategy selStrategy = stage == Q_ROOT_STAGE ? SelectionStrategy::SIMPLE : SelectionStrategy::STANDARD_ORDERING;

        move = moveSelector.selectNext<GenerationStrategy::STRICT, selStrategy>();
        while (move.see > 0) {
            // Delta pruning
            if (standpat + move.see + DELTA_MARGIN < alpha && moveSelector.currGenType != MoveGeneration::CHECK_EVASION) {
                if constexpr (trace) {
                    std::cout << std::dec << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: (" << move << ", see: ";
                    std::cout << move.see << ") failed delta pruning test\n";
                }

                if constexpr (stage == Q_ROOT_STAGE)
                    break;
                else {
                    move = moveSelector.selectNext<GenerationStrategy::STRICT, selStrategy>();
                    continue;
                }
            }

            if constexpr (trace) {
                std::cout << std::dec << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: checking " << move << "\n";
            }

            virtualBoard.makeMove(move);
            SearchResult result = quiescence<Q_COMMON_STAGE, trace>(-beta, -alpha, depth - 1);
            result.score = -result.score;   // For NegaMax purposes
            virtualBoard.undoLastMove();

            if constexpr (trace) {
                std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: " << move << " evaluated as ";
                std::cout << std::dec << relative_score(result.score, &virtualBoard) << "\n";
            }

            if (result.score >= beta)
                return {result.score, CUT_NODE, move};
                    
            if (result.score > bestScore) {
                bestScore = result.score;
                bestMove = move;
                if (result.score > alpha) {
                    alpha = result.score;
                    nodeType = PV_NODE;
                }
            }
                
            move = moveSelector.selectNext<GenerationStrategy::STRICT, selStrategy>();
        }

        if constexpr (trace) {
            std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: (" << move << ", see: ";
            std::cout << std::dec << move.see << ") failed see > 0 test\n";
        }
    }

    int noThreats = evaluator.getThreatCount(virtualBoard.movingSide());
    Bitboard threats = evaluator.getThreatMap(virtualBoard.movingSide());

    // Consider moving threatened pieces or capture attacking pieces to stabilize
    if (standpat + EPSILON_MARGIN > alpha && (noThreats > 1 || virtualBoard.isInCheck())) {
        if constexpr (trace)
            std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: position still not quiet, evaluating other moves...\n";

        if (move == Move::null())
            move = moveSelector.selectNext<GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE>();
        while (move != Move::null()) {
            PieceType pType = type_of(virtualBoard.onSquare(move.to()));
            Bitboard attacks = pType == NULL_TYPE ? 0 :
                               pType == PAWN ? Pieces::pawn_attacks(~virtualBoard.movingSide(), move.to()) :
                                               Pieces::piece_attacks_d(pType, move.to(), virtualBoard.pieces());
            if (virtualBoard.isInCheck() ||
                moveSelector.currGenType != MoveGeneration::CAPTURE && threats & move.from() ||
                moveSelector.currGenType == MoveGeneration::CAPTURE &&
                standpat + EPSILON_MARGIN + move.see > alpha &&
                (threats & move.from() || attacks & threats)
            ) {
                if constexpr (trace) {
                    std::cout << std::dec << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: checking " << move << "\n";
                }

                virtualBoard.makeMove(move);
                SearchResult result = quiescence<Q_COMMON_STAGE, trace>(-beta, -alpha, depth - 1);
                result.score = -result.score;
                virtualBoard.undoLastMove();

                if constexpr (trace) {
                    std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: " << move << " evaluated as ";
                    std::cout << std::dec << relative_score(result.score, &virtualBoard) << "\n";
                }

                if (result.score >= beta)
                    return {result.score, CUT_NODE, move};
                    
                if (result.score > bestScore) {
                    bestScore = result.score;
                    bestMove = move;
                    if (result.score > alpha) {
                        alpha = result.score;
                        nodeType = PV_NODE;
                    }
                }
            }
            else {
                if constexpr (trace)
                    std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: " << move << " omitted\n";
            }

            move = moveSelector.selectNext<GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE>();
        }

        return {bestScore, nodeType, bestMove};
    }

    if constexpr (trace)
        std::cout << "[QUEIESCENCE " << MAX_QUIESCENCE_DEPTH - depth << "]: position quiet enough, returning the score...\n";
    
    if (standpat > bestScore)
        return {standpat, standpat > alpha ? PV_NODE : ALL_NODE, standpat > alpha ? bestMove : Move::null()};
    else
        return {bestScore, nodeType, bestMove};
}