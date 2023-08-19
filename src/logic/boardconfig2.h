#pragma once

#include "progressstack.h"
#include "fenreader.h"
#include "observable.h"
#include <vector>

class BoardConfig2
{
public:
	BoardConfig2();
	virtual ~BoardConfig2();

	void addObserver(PositionChangedObserver* observer);
	void removeObserver(PositionChangedObserver* observer);
	void updateObserversByMove(int pieceID, const Square& oldPos, const Square& newPos);
private:


	std::vector<PositionChangedObserver*> positionObservers;
};