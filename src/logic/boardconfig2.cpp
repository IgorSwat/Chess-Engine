#include "boardconfig2.h"
#include <algorithm>

// Handling the position observers
void BoardConfig2::addObserver(PositionChangedObserver* observer)
{
	positionObservers.push_back(observer);
}

void BoardConfig2::removeObserver(PositionChangedObserver* observer)
{
	auto it = std::find(positionObservers.begin(), positionObservers.end(), observer);
	if (it != positionObservers.end())
		positionObservers.erase(it);
}

void BoardConfig2::updateObserversByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
	for (PositionChangedObserver* observer : positionObservers)
		observer->updateByMove(pieceID, oldPos, newPos);
}