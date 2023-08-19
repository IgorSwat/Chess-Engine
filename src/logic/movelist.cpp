#include "movelist.h"
using std::vector;

void MoveList::setKey(const Square& dir)
{
	if (keyCount < capacity)
	{
		keyMapping[KEY_STARTING_POINT + directionHash(dir)] = keyCount;
		keyCount++;
	}
}

void MoveList::reset(int directions)
{
	for (int i = 0; i < MAX_KEY_RANGE; i++)
		keyMapping[i] = 0;
	keyCount = 1;
	capacity = directions;
}