#include "test.h"
#include "../logic/boardElements.h"
#include "../logic/pieces.h"
#include "../logic/zobrist.h"


// -----------------
// Testing framework
// -----------------

bool Tester::environmentReady = false;

// Necessary preprocessing
void Tester::initEnvironment() const
{
    Pieces::initAttackTables();
	Board::initBoardElements();
	Zobrist::initZobristHashing();

    environmentReady = true;
}


namespace Testing {

}