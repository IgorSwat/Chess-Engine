#include "tuning.h"
#include <cstdlib>
#include <execution>
#include <future>
#include <numeric>
#include <memory>
#include <vector>


// ---------------------
// PgnEvalReader methods
// ---------------------

bool PgnEvalReader::processNext()
{
    if (!PgnParser::processNext())
        return false;

    engine->setPosition(board);
    Value realEval = parseEval();
    Value myEval = Search::relative_score(engine->iterativeDeepening(depth), board);
    
    totalNoMoves++;
    evalDifference += std::abs(realEval - myEval);

    return true;
}

void PgnEvalReader::reset()
{
    PgnParser::reset();
    evalDifference = 0;
}

Value PgnEvalReader::parseEval()
{
    std::size_t pos = pgn.find("eval", std::distance(pgn.begin(), currentPos));

    if (pos == std::string::npos)
        return 0;
    
    std::size_t evalEndPos = pgn.find(']', pos);
    std::string evalNotation = pgn.substr(pos + 5, evalEndPos - pos - 5);

    return Value(std::stod(evalNotation) * 100);
}

float PgnEvalReader::averageEvalDifference() const
{
    return float(evalDifference) / totalNoMoves;
}


// ---------------------
// Eval tuning functions
// ---------------------

void tune(int noGames, Search::Depth maxDepth)
{
    std::uint64_t totalEvalDiff = 0;
    std::uint32_t totalNoMoves = 0;

    std::vector<std::pair<std::uint64_t, std::uint32_t>> results(noGames);

    // Równoległe przetwarzanie gier
    std::for_each(std::execution::par, results.begin(), results.end(), [&](auto& result) {
        std::unique_ptr<BoardConfig> board = std::make_unique<BoardConfig>();
        std::unique_ptr<Engine> engine = std::make_unique<Engine>();

        int i = &result - &results[0] + 1; // indeks gry
        std::string filepath = "testpos/game" + std::to_string(i) + ".pgn";

        PgnEvalReader reader(filepath, board.get(), engine.get(), maxDepth);
        reader.processAll();

        result.first = reader.evalDifference;
        result.second = reader.totalNoMoves;
    });

    // Sumowanie wyników po zakończeniu wszystkich wątków
    for (const auto& [evalDiff, noMoves] : results) {
        totalEvalDiff += evalDiff;
        totalNoMoves += noMoves;
    }

    std::cout << "Average eval difference: " << float(totalEvalDiff) / totalNoMoves << "\n";
}