#include "test.h"
#include "../logic/boardConfig.h"
#include "../engine/moveGeneration.h"
#include <iterator>
#include <algorithm>
#include <fstream>
#include <chrono>

namespace Testing {

	struct MoveCounters
	{
		uint64_t noMoves = 0;
		uint64_t noCaptures = 0;
		uint64_t noEnpassants = 0;
		uint64_t noCastles = 0;
		uint64_t noPromotions = 0;

		void operator+=(const MoveCounters &other)
		{
			noMoves += other.noMoves;
			noCaptures += other.noCaptures;
			noEnpassants += other.noEnpassants;
			noCastles += other.noCastles;
			noPromotions += other.noPromotions;
		}
	};

	struct PerftData
	{
		std::string fen;
		int depth;
		MoveCounters moveCounters;
	};

	// Full perft function to check in details correctness of generated moves
	template <bool trace>
	MoveCounters perftFull(BoardConfig& board, int depth, std::ostream& output, std::vector<Move>* moveTrace)
	{
		MoveList generatedMoves;
		MoveCounters moveCounters = { 0, 0, 0, 0, 0 };

		MoveGeneration::generateMoves<MoveGeneration::LEGAL>(generatedMoves, board);
		if (depth == 1) {
			if constexpr (trace) {
				output << "Path to the node:\n";
				for (const Move& move : *moveTrace)
					output << move << std::endl;
				output << " Leaves from node:\n";
				output << generatedMoves << std::endl;
			}
			moveCounters.noMoves = generatedMoves.size();
			moveCounters.noCaptures = std::count_if(generatedMoves.begin(), generatedMoves.end(), [](const Move& move) {return move.isCapture(); });
			moveCounters.noEnpassants = std::count_if(generatedMoves.begin(), generatedMoves.end(), [](const Move& move) {return move.isEnpassant(); });
			moveCounters.noCastles = std::count_if(generatedMoves.begin(), generatedMoves.end(), [](const Move& move) {return move.isCastle(); });
			moveCounters.noPromotions = std::count_if(generatedMoves.begin(), generatedMoves.end(), [](const Move& move) {return move.isPromotion(); });
		}
		else {
			for (const Move& move : generatedMoves) {
				if constexpr (trace)
					moveTrace->push_back(move);
				board.makeMove(move);
				MoveCounters subcounters = perftFull<trace>(board, depth - 1, output, moveTrace);
				if constexpr (trace)
					moveTrace->pop_back();
				board.undoLastMove();
				moveCounters += subcounters;
			}
		}
		return moveCounters;
	}

	// Lightweight perft function to measure the performance of move generation
	uint64_t perftLight(BoardConfig& board, int depth)
	{
		if (depth == 0) return 1;

		MoveList generatedMoves;
		uint64_t noMoves = 0;

		MoveGeneration::generateMoves<MoveGeneration::LEGAL>(generatedMoves, board);
		for (const Move& move : generatedMoves) {
			board.makeMove(move);
			noMoves += perftLight(board, depth - 1);
			board.undoLastMove();
		}
		return noMoves;
	}

	void perftMovegenTest(const PerftData& data, bool trace = false)
	{
		BoardConfig board;
		board.loadFromFen(data.fen);
		MoveCounters results;
		if (trace) {
			std::ofstream output("perftDebug.txt");
			std::vector<Move>* moveTrace = new std::vector<Move>();
			results = perftFull<true>(board, data.depth, output, moveTrace);
			delete moveTrace;
			output.close();
		}
		else
			results = perftFull<false>(board, data.depth, std::cout, nullptr);

		assert(results.noPromotions == data.moveCounters.noPromotions);
		assert(results.noCastles == data.moveCounters.noCastles);
		assert(results.noEnpassants == data.moveCounters.noEnpassants);
		assert(results.noCaptures == data.moveCounters.noCaptures);
		assert(results.noMoves == data.moveCounters.noMoves);
	}

	void perftSpeedTest(const PerftData& data)
	{
		BoardConfig board;
		board.loadFromFen(data.fen);

		auto startTime = std::chrono::steady_clock::now();
		uint64_t result = perftLight(board, data.depth);
		auto endTime = std::chrono::steady_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
		std::cout << "Time taken for fen " << data.fen << " and depth " << data.depth << ":\n";
		std::cout << duration.count() << " seconds\n";

		assert(result == data.moveCounters.noMoves);
	}

	void perftMovegenTestStartingPos()
	{
		const PerftData data2 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
								 2, {400, 0, 0, 0, 0} };
		perftMovegenTest(data2);

		const PerftData data3 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
								 3, {8902, 34, 0, 0, 0} };
		perftMovegenTest(data3);

		const PerftData data4 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
								 4, {197281, 1576, 0, 0, 0} };
		perftMovegenTest(data4);

		const PerftData data5 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
								 5, {4865609, 82719, 258, 0, 0} };
		perftMovegenTest(data5);
	}

	void perftMovegenTestMidgamePos1()
	{
		const PerftData data1 = { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
								 1, {48, 8, 0, 2, 0} };
		perftMovegenTest(data1);

		const PerftData data2 = { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
								 2, {2039, 351, 1, 91, 0} };
		perftMovegenTest(data2);

		const PerftData data3 = { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
								 3, {97862, 17102, 45, 3162, 0} };
		perftMovegenTest(data3);

		const PerftData data4 = { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
								 4, {4085603, 757163, 1929, 128013, 15172} };
		perftMovegenTest(data4);
	}

	void perftMovegenTestMidgamePos2()
	{
		const PerftData data1 = { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
								 1, {6, 0, 0, 0, 0} };
		perftMovegenTest(data1);

		const PerftData data2 = { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
								 2, {264, 87, 0, 6, 48} };
		perftMovegenTest(data2);

		const PerftData data3 = { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
								 3, {9467, 1021, 4, 0, 120} };
		perftMovegenTest(data3);

		const PerftData data4 = { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
								 4, {422333, 131393, 0, 7795, 60032} };
		perftMovegenTest(data4);

		const PerftData data5 = { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
								 5, {15833292, 2046173, 6512, 0, 329464} };
		perftMovegenTest(data5);
	}

	void perftMovegenTestMidgamePos3()
	{
		const PerftData data2 = { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
								 2, {1486, 222, 0, 0, 0} };
		perftMovegenTest(data2);

		const PerftData data3 = { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
								 3, {62379, 8517, 0, 1081, 5068} };
		perftMovegenTest(data3);

		const PerftData data4 = { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
								 4, {2103487, 296153, 0, 0, 0} };
		perftMovegenTest(data4);
	}

	void perftMovegenTestEndgamePos()
	{
		const PerftData data1 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 1, {14, 1, 0, 0, 0} };
		perftMovegenTest(data1);

		const PerftData data2 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 2, {191, 14, 0, 0, 0} };
		perftMovegenTest(data2);

		const PerftData data3 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 3, {2812, 209, 2, 0, 0} };
		perftMovegenTest(data3);

		const PerftData data4 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 4, {43238, 3348, 123, 0, 0} };
		perftMovegenTest(data4);

		const PerftData data5 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 5, {674624, 52051, 1165, 0, 0} };
		perftMovegenTest(data5);

		const PerftData data6 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 6, {11030083, 940350, 33325, 0, 7552} };
		perftMovegenTest(data6);
	}

	template <bool deep>
	void perftSpeedTestStartingPos()
	{
		if constexpr (!deep) {
			const PerftData data5 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
								 5, {4865609	, 0, 0, 0, 0} };
			perftSpeedTest(data5);
		}
		if constexpr (deep) {
			const PerftData data6 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
								 6, {119060324, 0, 0, 0, 0} };
			perftSpeedTest(data6);
		}

	}

	template void perftSpeedTestStartingPos<false>();
	template void perftSpeedTestStartingPos<true>();

	template <bool deep>
	void perftSpeedTestMidgamePos()
	{
		if constexpr (!deep) {
			const PerftData data5 = { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
								 5, {15833292, 0, 0, 0, 0} };
			perftSpeedTest(data5);
		}
		if constexpr (deep) {
			const PerftData data6 = { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
								 6, {706045033, 0, 0, 0, 0} };
			perftSpeedTest(data6);
		}
	}

	template void perftSpeedTestMidgamePos<false>();
	template void perftSpeedTestMidgamePos<true>();

	template <bool deep>
	void perftSpeedTestEndgamePos()
	{
		if constexpr (!deep) {
			const PerftData data6 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 6, {11030083, 0, 0, 0, 0} };
			perftSpeedTest(data6);
		}
		if constexpr (deep) {
			const PerftData data7 = { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
								 7, {178633661, 0, 0, 0, 0} };
			perftSpeedTest(data7);
		}
	}

	template void perftSpeedTestEndgamePos<false>();
	template void perftSpeedTestEndgamePos<true>();
}