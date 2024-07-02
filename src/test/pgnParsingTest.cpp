#include "test.h"
#include "../utilities/pgnParser.h"


namespace Testing {

    // ------------------------
	// PGN parsing tester class
	// ------------------------

    class PgnParsingTester : public PgnParser
    {
    public:
        PgnParsingTester(const std::string& pgnFilePath, BoardConfig* board, std::initializer_list<Move> expectedMoves)
            : PgnParser(pgnFilePath, board), expectedMoves(expectedMoves), currentMoveIt(this->expectedMoves.begin()) {}

        bool processNext() override
        {
            if (currentMoveIt != expectedMoves.end())
            {
                assert(parseToNextMove());
                std::string notation = parseMove();
                Move move = moveFromNotation(notation);

                assert(move == *currentMoveIt);
                std::cout << "Succesfully parsed: " << move << "\n";

                board->makeMove(move);
                currentMoveIt++;
                return true;
            }
            else
                return false;
        }

        void reset() override
        {
            PgnParser::reset();
            currentMoveIt = expectedMoves.begin();
        }

    private:
        std::initializer_list<Move> expectedMoves;
        std::initializer_list<Move>::const_iterator currentMoveIt;
    };


    // --------------
	// Test functions
	// --------------

    void pgnParsingTest()
    {
        const std::string filePath = "testpos/pgn-parsing-test.pgn";

        const std::initializer_list<Move> expectedMoves = {
            Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_FLAG),
            Move(SQ_C7, SQ_C5, DOUBLE_PAWN_PUSH_FLAG),
            Move(SQ_G1, SQ_F3, QUIET_MOVE_FLAG),
            Move(SQ_D7, SQ_D6, QUIET_MOVE_FLAG),
            Move(SQ_D2, SQ_D4, DOUBLE_PAWN_PUSH_FLAG),
            Move(SQ_C5, SQ_D4, CAPTURE_FLAG),
            Move(SQ_F3, SQ_D4, CAPTURE_FLAG),
            Move(SQ_G8, SQ_F6, QUIET_MOVE_FLAG),
            Move(SQ_B1, SQ_C3, QUIET_MOVE_FLAG),
            Move(SQ_A7, SQ_A6, QUIET_MOVE_FLAG),
            Move(SQ_F2, SQ_F4, DOUBLE_PAWN_PUSH_FLAG),
            Move(SQ_G7, SQ_G6, QUIET_MOVE_FLAG),
            Move(SQ_F4, SQ_F5, QUIET_MOVE_FLAG),
            Move(SQ_E7, SQ_E5, DOUBLE_PAWN_PUSH_FLAG),
            Move(SQ_F5, SQ_E6, ENPASSANT_FLAG),
            Move(SQ_F8, SQ_E7, QUIET_MOVE_FLAG),
            Move(SQ_E6, SQ_F7, CAPTURE_FLAG),
            Move(SQ_E8, SQ_D7, QUIET_MOVE_FLAG),
            Move(SQ_F7, SQ_F8, KNIGHT_PROMOTION_FLAG),
            Move(SQ_D7, SQ_C7, QUIET_MOVE_FLAG),
            Move(SQ_F8, SQ_E6, QUIET_MOVE_FLAG),
            Move(SQ_C7, SQ_B6, QUIET_MOVE_FLAG),
            Move(SQ_E6, SQ_F4, QUIET_MOVE_FLAG),
            Move(SQ_A6, SQ_A5, QUIET_MOVE_FLAG),
            Move(SQ_C1, SQ_E3, QUIET_MOVE_FLAG),
            Move(SQ_A8, SQ_A6, QUIET_MOVE_FLAG),
            Move(SQ_C3, SQ_D5, QUIET_MOVE_FLAG),
            Move(SQ_B6, SQ_A7, QUIET_MOVE_FLAG),
            Move(SQ_D5, SQ_B6, QUIET_MOVE_FLAG),
            Move(SQ_D8, SQ_C7, QUIET_MOVE_FLAG),
            Move(SQ_B6, SQ_D7, QUIET_MOVE_FLAG),
            Move(SQ_H8, SQ_G8, QUIET_MOVE_FLAG),
            Move(SQ_D7, SQ_C5, QUIET_MOVE_FLAG),
            Move(SQ_G8, SQ_H8, QUIET_MOVE_FLAG),
            Move(SQ_C5, SQ_E6, QUIET_MOVE_FLAG),
            Move(SQ_H8, SQ_G8, QUIET_MOVE_FLAG),
            Move(SQ_E6, SQ_D8, QUIET_MOVE_FLAG),
            Move(SQ_F6, SQ_E4, CAPTURE_FLAG),
            Move(SQ_D4, SQ_E6, QUIET_MOVE_FLAG),
            Move(SQ_B7, SQ_B6, QUIET_MOVE_FLAG),
            Move(SQ_D1, SQ_D2, QUIET_MOVE_FLAG),
            Move(SQ_H7, SQ_H5, DOUBLE_PAWN_PUSH_FLAG),
            Move(SQ_E1, SQ_C1, QUEENSIDE_CASTLE_FLAG)
        };

        BoardConfig board;
        PgnParsingTester tester(filePath, &board, expectedMoves);

        tester.processAll();
    }

}