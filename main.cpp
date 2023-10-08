#include "src/logic/boardConfig.h"
#include <iostream>
#include <iomanip>
#include <SFML/Graphics.hpp>
using namespace std;
	
int main()
{
	BoardConfig config;
	config.makeMove(Move(SQ_E2, SQ_E4, DOULBLE_PAWN_PUSH_FLAG));
	config.makeMove(Move(SQ_E7, SQ_E5, DOULBLE_PAWN_PUSH_FLAG));
	config.makeMove(Move(SQ_E1, SQ_E2, QUIET_MOVE));
	cout << config << endl;

	return 0;
}