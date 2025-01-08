#include "pgnParser.h"
#include "../engine/engine.h"


// ---------------
// PGN eval reader
// ---------------

namespace Utilities {

    // ------------------
    // Engine eval parser
    // ------------------

    namespace Parsing {

        class PgnEvalReader : public PgnParser
        {
        public:
            PgnEvalReader(const std::string &pgnFilePath, BoardConfig *board, Engine *engine, Search::Depth maxDepth = 6)
                : PgnParser(pgnFilePath, board), engine(engine), depth(maxDepth) {}

            bool processNext() override;
            void reset() override;

            float averageEvalDifference() const;

            int evalDifference = 0;
            int totalNoMoves = 0;

        private:
            Value parseEval();

            Engine *engine;
            const Search::Depth depth;
        };
    }

    // ---------------------
    // Eval tuning functions
    // ---------------------

    void tune(int noGames, Search::Depth maxDepth);
}