#include "test.h"
#include "../utilities/pgnParser.h"
#include "../logic/boardConfig.h"
#include "../logic/zobrist.h"
#include <iostream>


namespace Testing {

	// --------------------
	// Zobrist tester class
	// --------------------

	class PgnZobristTester : public Utilities::Parsing::PgnParser
	{
	public:
		PgnZobristTester(const std::string& pgnFilePath, BoardConfig* board) : Utilities::Parsing::PgnParser(pgnFilePath, board) {}

		bool processNext() override {
			// Try to load new move
			if (!Utilities::Parsing::PgnParser::processNext())
				return false;

			// Compare statically and dynamically generated zobrist hash
			zobrist.generateHash(board);
			assert(zobrist.getHash() == board->hash());
			return true;
		}

	private:
		Zobrist::ZobristHash zobrist;
	};


	// --------------
	// Test functions
	// --------------


    void zobristTest()
	{
		// PGN file paths
		const auto testCases = {
			"testpos/zobrist-test-1.pgn",
			"testpos/zobrist-test-2.pgn"
		};

		for (const std::string& filePath : testCases) {
			BoardConfig board;
			PgnZobristTester tester(filePath, &board);
			tester.processAll();
		}
	}

}