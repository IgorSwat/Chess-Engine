#include "pawnstructure.h"
#include <algorithm>
#include <functional>
// Remember about negative factor!

constexpr int PawnStructure::columnsSide[];
constexpr int PawnStructure::columnsDistanceWages[];
constexpr int BackwardPawns::endValues[];
constexpr int BackwardPawns::rowDiff[];
constexpr int BackwardPawns::backwardLowerBounds[];
constexpr int BackwardPawns::backwardUpperBounds[];
constexpr int BackwardPawns::excludedRows[];


namespace {
    using vecIter = std::vector<int>::const_iterator;

    constexpr int bounds[2] { 0, 7 };
    constexpr int frontSpawns[2] { -1, 1 };
    constexpr int blockerLowerBounds[2] { 3, 0 };
    constexpr int blockerUpperBounds[2] { 7, 4 };

    std::function<vecIter(std::vector<int>&)> boundaryFunctions[2]{
        [](std::vector<int>& vec)->vecIter {return std::max_element(vec.begin(), vec.end()); },
        [](std::vector<int>& vec)->vecIter {return std::min_element(vec.begin(), vec.end()); }
    };
    Comparator isBottomRow[2]{ [](const int& targetRow, const int& comparisionRow)->bool {return targetRow > comparisionRow; },
                               [](const int& targetRow, const int& comparisionRow)->bool {return targetRow < comparisionRow; } };

    DistanceCalculator passedDistance[2]{ [](const int& row)->int {return row; },
                                          [](const int& row)->int {return 7 - row; } };

}



/// PawnStructure methods
PawnStructure::PawnStructure(BoardConfig* cnf, MaterialBalance* mt, SquareControl* ctr, const FactorsVec& ftors) : 
    PositionElement("PawnStructure", ftors, 0), config(cnf), material(mt), control(ctr)
{
    initTables();

    IsolatedPawns* isolated = new IsolatedPawns(this);
    DoubledPawns* doubled = new DoubledPawns(this);
    PawnIslands* islands = new PawnIslands(this);
    BackwardPawns* backwards = new BackwardPawns(this);
    passed = new PassedPawns(this, config, control);
    columnWatchers.push_back(isolated);
    columnWatchers.push_back(doubled);
    columnWatchers.push_back(islands);
    generalWatchers.push_back(backwards);
    generalWatchers.push_back(passed);
}

PawnStructure::~PawnStructure()
{
    for (PawnsWatcher* watcher : generalWatchers)
        delete watcher;
    for (PawnsWatcher* watcher : columnWatchers)
        delete watcher;
}

void PawnStructure::resetSubelements()
{
    for (PawnsWatcher* element : columnWatchers)
        element->clearTables();
    for (PawnsWatcher* element : generalWatchers)
        element->clearTables();
}

void PawnStructure::updateWatchersByInsertion(const vector<PawnsWatcher*>& watchers, const Side& side, const int& row, const int& col)
{
    for (PawnsWatcher* watcher : watchers)
        watcher->updateByInsertion(side, row, col);
}

void PawnStructure::updateWatchersByRemoval(const vector<PawnsWatcher*>& watchers, const Side& side, const int& row, const int& col)
{
    for (PawnsWatcher* watcher : watchers)
        watcher->updateByRemoval(side, row, col);
}

void PawnStructure::clearTables()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            pawnsAtColumns[i][j].clear();
            bottomPawns[i][0] = 0;
            bottomPawns[i][1] = 7;
            upperPawns[i][0] = 7;
            upperPawns[i][1] = 0;
            for (int k = 0; k < 8; k++)
                pawnsMap[i][k][j] = false;
        }
    }
    pawnsAtColors[0][0] = pawnsAtColors[0][1] = 0;
    pawnsAtColors[1][0] = pawnsAtColors[1][1] = 0;
    distantPawnsSum[0][0] = distantPawnsSum[0][1] = 0;
    distantPawnsSum[1][0] = distantPawnsSum[1][1] = 0;
    pawnRowSums[0][0] = pawnRowSums[0][1] = 0;
    pawnRowSums[1][0] = pawnRowSums[1][1] = 0;
}

void PawnStructure::addPawn(const Side& side, const int& row, const int& col)
{
    pawnsMap[row][col][side] = true;
    pawnsAtColors[(row + col) % 2][side] += 1;
    pawnsAtColumns[col][side].push_back(row);
    pawnRowSums[colorsMap[row][col]][side] += row;
    if (countPawnsAtColumn(side, col) == 1)
    {
        distantPawnsSum[side][columnsSide[col]] += columnsDistanceWages[col];
    }
    if (side == WHITE)
    {
        if (row > bottomPawns[col][0]) bottomPawns[col][0] = row;
        if (row < upperPawns[col][0]) upperPawns[col][0] = row;
    }
    else
    {
        if (row < bottomPawns[col][1]) bottomPawns[col][1] = row;
        if (row > upperPawns[col][1]) upperPawns[col][1] = row;
    }
}

void PawnStructure::removePawn(const Side& side, const int& row, const int& col)
{
    std::vector<int>& columnData = pawnsAtColumns[col][side];
    pawnsMap[row][col][side] = false;
    pawnsAtColors[(row + col) % 2][side] -= 1;
    columnData.erase(std::find(columnData.begin(), columnData.end(), row));
    pawnRowSums[colorsMap[row][col]][side] -= row;
    bool isColumnEmpty = countPawnsAtColumn(side, col) == 0;
    if (isColumnEmpty)
    {
        distantPawnsSum[side][columnsSide[col]] -= columnsDistanceWages[col];
    }
    if (row == bottomPawns[col][side])
        bottomPawns[col][side] = isColumnEmpty ? bounds[side] : (*boundaryFunctions[side](columnData));
    if (row == upperPawns[col][side])
    {
        Side opposite = opposition[side];
        upperPawns[col][side] = isColumnEmpty ? bounds[(int)opposite] : (*boundaryFunctions[(int)opposite](columnData));
    }
}

void PawnStructure::reset()
{
    clearTables();
    resetSubelements();
    for (int i = 0; i < 32; i++)
    {
        const Piece* piece = config->getPiece(i);
        if (piece->isActive() && piece->getType() == PAWN)
        {
            const Square& pos = piece->getPosition();
            Side color = piece->getColor();
            addPawn(color, pos.y, pos.x);
            updateWatchersByInsertion(columnWatchers, color, pos.y, pos.x);
            updateWatchersByInsertion(generalWatchers, color, pos.y, pos.x);
        }

    }
}

void PawnStructure::updateByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
    Side side = pieceID < 16 ? WHITE : BLACK;
    bool columnChange = oldPos.x != newPos.x;
    bool isStillAPawn = config->getPiece(pieceID)->getType() == PAWN;
    bool wasRegistered = pawnsMap[oldPos.y][oldPos.x][side];
    if (oldPos.x < 8 && wasRegistered)
    {
        removePawn(side, oldPos.y, oldPos.x);
        updateWatchersByRemoval(generalWatchers, side, oldPos.y, oldPos.x);
        if (columnChange || !isStillAPawn)
            updateWatchersByRemoval(columnWatchers, side, oldPos.y, oldPos.x);
    }
    else if (oldPos.x < 8 && !isStillAPawn)
        passed->updateByPiecePositionChanged(side, config->getPiece(pieceID)->getType(), oldPos.y, oldPos.x, PassedPawns::MULTIPLICATE);
    if (newPos.x < 8 && isStillAPawn)
    {
        addPawn(side, newPos.y, newPos.x);
        updateWatchersByInsertion(generalWatchers, side, newPos.y, newPos.x);
        if (columnChange || !wasRegistered)
            updateWatchersByInsertion(columnWatchers, side, newPos.y, newPos.x);
    }
    else if (newPos.x < 8 && !wasRegistered)
        passed->updateByPiecePositionChanged(side, config->getPiece(pieceID)->getType(), newPos.y, newPos.x, PassedPawns::DIVIDE);
}

bool PawnStructure::distantPawnsCheck(Side side) const
{
    return distantPawnsSum[side][0] > unitCondition && distantPawnsSum[side][1] > unitCondition &&
        distantPawnsSum[side][0] + distantPawnsSum[side][1] > sumCondition;
}

void PawnStructure::show() const
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (pawnsMap[i][j][0])
                std::cout<<"0";
            else if (pawnsMap[i][j][1])
                std::cout<<"1";
            else
                std::cout<<"-";
        }
        std::cout<<std::endl;
    }
    std::cout<<"White pawns on light squares: "<<pawnsAtColors[0][0]<<std::endl;
    std::cout<<"White pawns on dark squares: "<< pawnsAtColors[1][0]<<std::endl;
    std::cout<<"Black pawns on light squares: "<< pawnsAtColors[0][1]<<std::endl;
    std::cout<<"Black pawns on dark squares: "<< pawnsAtColors[1][1]<<std::endl;
    std::cout<<"White bottom pawns: ";
    for (int i = 0; i < 8; i++)
        std::cout<<bottomPawns[i][0]<<" ";
    std::cout<<std::endl;
    std::cout<<"Black bottom pawns: ";
    for (int i = 0; i < 8; i++)
        std::cout<<bottomPawns[i][1]<<" ";
    std::cout << "\nWhite upper pawns: ";
    for (int i = 0; i < 8; i++)
        std::cout << upperPawns[i][0] << " ";
    std::cout << "\nBlack upper pawns: ";
    for (int i = 0; i < 8; i++)
        std::cout << upperPawns[i][1] << " ";
    std::cout << "\nWhite distant pawns check: " << distantPawnsCheck(WHITE)<< " " << std::endl;
    std::cout << "Black distant pawns check: " << distantPawnsCheck(BLACK) << " " << std::endl;
    std::cout<<std::endl;
    for (PawnsWatcher* element : columnWatchers)
        element->show();
    for (PawnsWatcher* element : generalWatchers)
        element->show();
}




/// IsolatedPawns methods
void IsolatedPawns::clearTables()
{
    isolatedPawns[0] = isolatedPawns[1] = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
            columnsIsolation[i][j] = 0;
    }
}

void IsolatedPawns::updateByInsertion(Side side, int row, int col)
{
    if (positioning->countPawnsAtColumn(side, col) == 1)
    {
        if (col > 0)
        {
            if (columnsIsolation[col - 1][side] == 0)
                isolatedPawns[side] -= positioning->countPawnsAtColumn(side, col - 1) * positioning->isolatedPawnColumnPoints[col - 1];
            columnsIsolation[col - 1][side] += 1;
        }
        if (col < 7)
        {
            if (columnsIsolation[col + 1][side] == 0)
                isolatedPawns[side] -= positioning->countPawnsAtColumn(side, col + 1) * positioning->isolatedPawnColumnPoints[col + 1];
            columnsIsolation[col + 1][side] += 1;
        }
    }
    if (columnsIsolation[col][side] == 0)
        isolatedPawns[side] += positioning->isolatedPawnColumnPoints[col];
}

void IsolatedPawns::updateByRemoval(Side side, int row, int col)
{
    if (positioning->countPawnsAtColumn(side, col) == 0)
    {
        if (col > 0)
        {
            columnsIsolation[col - 1][side] -= 1;
            if (columnsIsolation[col - 1][side] == 0)
                isolatedPawns[side] += positioning->countPawnsAtColumn(side, col - 1) * positioning->isolatedPawnColumnPoints[col - 1];
        }
        if (col < 7)
        {
            columnsIsolation[col + 1][side] -= 1;
            if (columnsIsolation[col + 1][side] == 0)
                isolatedPawns[side] += positioning->countPawnsAtColumn(side, col + 1) * positioning->isolatedPawnColumnPoints[col + 1];
        }
    }
    if (columnsIsolation[col][side] == 0)
        isolatedPawns[side] -= positioning->isolatedPawnColumnPoints[col];
}




/// DoubledPawns methods
void DoubledPawns::updateByInsertion(Side side, int row, int col)
{
    int pawnsCount = positioning->countPawnsAtColumn(side, col);
    doubledPawns[side] -= numberOfPairs(pawnsCount - 1);
    doubledPawns[side] += numberOfPairs(pawnsCount);
}

void DoubledPawns::updateByRemoval(Side side, int row, int col)
{
    int pawnsCount = positioning->countPawnsAtColumn(side, col);
    doubledPawns[side] -= numberOfPairs(pawnsCount + 1);
    doubledPawns[side] += numberOfPairs(pawnsCount);
}




/// PawnIslands methods
void PawnIslands::updateByInsertion(Side side, int row, int col)
{
    if (positioning->countPawnsAtColumn(side, col) == 1)
    {
        bool leftFlag = col > 0 && positioning->countPawnsAtColumn(side, col - 1) > 0;
        bool rightFlag = col < 7 && positioning->countPawnsAtColumn(side, col + 1) > 0;
        if (leftFlag && rightFlag)
            pawnIslands[side] -= 1;
        else if (!leftFlag && !rightFlag)
            pawnIslands[side] += 1;
    }
}

void PawnIslands::updateByRemoval(Side side, int row, int col)
{
    if (positioning->countPawnsAtColumn(side, col) == 0)
    {
        bool leftFlag = col > 0 && positioning->countPawnsAtColumn(side, col - 1) > 0;
        bool rightFlag = col < 7 && positioning->countPawnsAtColumn(side, col + 1) > 0;
        if (leftFlag && rightFlag)
            pawnIslands[side] += 1;
        else if (!leftFlag && !rightFlag)
            pawnIslands[side] -= 1;
    }
}




/// BackwardPawns methods
void BackwardPawns::clearTables()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                adjecentFilesPropertyCount[i][k] = 0;
                pathControlledByEnemyPawn[i][j][k] = 0;
                backwardPawnRows[i][k] = endValues[k];
            }
        }
    }
    backwardPawns[0] = backwardPawns[1] = 0;
    backwardPawnsAtSemiopenFiles[0] = backwardPawnsAtSemiopenFiles[1] = 0;
}

void BackwardPawns::updateAdjecentFilesCounter(Side side, int col)
{
    adjecentFilesPropertyCount[col][side] = 0;
    adjecentFilesPropertyCount[col][side] += (col == 0 || isBottomRow[side](positioning->bottomPawns[col][side], positioning->bottomPawns[col - 1][side]));
    adjecentFilesPropertyCount[col][side] += (col == 7 || isBottomRow[side](positioning->bottomPawns[col][side], positioning->bottomPawns[col + 1][side]));
}

void BackwardPawns::updateOpponentPawnsOnAdjecentFileWhenAddingPawn(Side side, Side opposite, int col, int oppositeRow)
{
    pathControlledByEnemyPawn[oppositeRow][col][(int)opposite] += 1;
    if (pathControlledByEnemyPawn[oppositeRow][col][(int)opposite] == 1 && oppositeRow == positioning->bottomPawns[col][(int)opposite] &&
     adjecentFilesPropertyCount[col][(int)opposite] == 2 && oppositeRow != excludedRows[opposite])
    {
        updateBackwardCounters(opposite, side, col, 1);
        backwardPawnRows[col][(int)opposite] = oppositeRow;
    }
}

void BackwardPawns::updateOwnPawnsOnAdjecentFileWhenAddingPawn(Side side, Side opposite, int col)
{
    int bottomSquare = positioning->bottomPawns[col][side];
    if (bottomSquare != endValues[side])
    {
        updateAdjecentFilesCounter(side, col);
        if (backwardPawnRows[col][side] == bottomSquare && adjecentFilesPropertyCount[col][side] < 2)
        {
            updateBackwardCounters(side, opposite, col, -1);
            backwardPawnRows[col][side] = endValues[side];
        }
    }
}

void BackwardPawns::updateByInsertion(Side side, int row, int col)
{
    Side opposite = opposition[side];
    if (row >= backwardLowerBounds[side] && row <= backwardUpperBounds[side])
    {
        int oppositeRow = row + rowDiff[side];
        if (row == excludedRows[side] || row != positioning->bottomPawns[col][side])
        {
            if (col > 0)
                updateOpponentPawnsOnAdjecentFileWhenAddingPawn(side, opposite, col - 1, oppositeRow);
            if (col < 7)
                updateOpponentPawnsOnAdjecentFileWhenAddingPawn(side, opposite, col + 1, oppositeRow);
            if (backwardPawnRows[col][side] != endValues[side] && positioning->countPawnsAtColumn(side, col) == 2 &&
             positioning->isFileSemiOpen(opposite, col))
                backwardPawnsAtSemiopenFiles[side] -= 1;
        }
        else
        {
            if (col > 0)
            {
                updateOpponentPawnsOnAdjecentFileWhenAddingPawn(side, opposite, col - 1, oppositeRow);
                updateOwnPawnsOnAdjecentFileWhenAddingPawn(side, opposite, col - 1);
            }
            if (col < 7)
            {
                updateOpponentPawnsOnAdjecentFileWhenAddingPawn(side, opposite, col + 1, oppositeRow);
                updateOwnPawnsOnAdjecentFileWhenAddingPawn(side, opposite, col + 1);
            }
            updateAdjecentFilesCounter(side, col);
            if (adjecentFilesPropertyCount[col][side] == 2 && pathControlledByEnemyPawn[row][col][side] > 0)
            {
                if (backwardPawnRows[col][side] == endValues[side])
                    updateBackwardCounters(side, opposite, col, 1);
                backwardPawnRows[col][side] = row;
            }
            else if (backwardPawnRows[col][side] != endValues[side])
            {
                updateBackwardCounters(side, opposite, col, -1);
                backwardPawnRows[col][side] = endValues[side];
            }
        }
    }
    if (positioning->countPawnsAtColumn(side, col) == 1 && backwardPawnRows[col][opposite] != endValues[opposite])
        backwardPawnsAtSemiopenFiles[opposite] -= 1;
}

void BackwardPawns::updateOpponentPawnsOnAdjecentFileWhenRemovingPawn(Side side, Side opposite, int col, int oppositeRow)
{
    pathControlledByEnemyPawn[oppositeRow][col][(int)opposite] -= 1;
    if (pathControlledByEnemyPawn[oppositeRow][col][(int)opposite] == 0 && backwardPawnRows[col][(int)opposite] == oppositeRow)
    {
        updateBackwardCounters(opposite, side, col, -1);
        backwardPawnRows[col][(int)opposite] = endValues[(int)opposite];
    }
}

void BackwardPawns::updateOwnPawnsOnAdjecentFileWhenRemovingPawn(Side side, Side opposite, int col)
{
    int bottomSquare = positioning->bottomPawns[col][side];
    if (bottomSquare != endValues[side])
    {
        updateAdjecentFilesCounter(side, col);
        if (backwardPawnRows[col][side] != bottomSquare && adjecentFilesPropertyCount[col][side] == 2 && 
         pathControlledByEnemyPawn[bottomSquare][col][side] > 0 && bottomSquare != excludedRows[side])
        {
            updateBackwardCounters(side, opposite, col, 1);
            backwardPawnRows[col][side] = bottomSquare;
        }
    }
}

void BackwardPawns::updateByRemoval(Side side, int row, int col)
{
    Side opposite = opposition[side];
    if (row >= backwardLowerBounds[side] && row <= backwardUpperBounds[side])
    {
        int oppositeRow = row + rowDiff[side];
        if (row == excludedRows[side] || !isBottomRow[side](row, positioning->bottomPawns[col][side]))
        {
            if (col > 0)
                updateOpponentPawnsOnAdjecentFileWhenRemovingPawn(side, opposite, col - 1, oppositeRow);
            if (col < 7)
                updateOpponentPawnsOnAdjecentFileWhenRemovingPawn(side, opposite, col + 1, oppositeRow);
            if (backwardPawnRows[col][side] != endValues[side] && positioning->countPawnsAtColumn(side, col) == 1 &&
             positioning->isFileSemiOpen(opposite, col))
                backwardPawnsAtSemiopenFiles[side] += 1;
        }
        else
        {
            if (col > 0)
            {
                updateOpponentPawnsOnAdjecentFileWhenRemovingPawn(side, opposite, col - 1, oppositeRow);
                updateOwnPawnsOnAdjecentFileWhenRemovingPawn(side, opposite, col - 1);
            }
            if (col < 7)
            {
                updateOpponentPawnsOnAdjecentFileWhenRemovingPawn(side, opposite, col + 1, oppositeRow);
                updateOwnPawnsOnAdjecentFileWhenRemovingPawn(side, opposite, col + 1);
            }
            if (backwardPawnRows[col][side] == row)
            {
                backwardPawns[side] -= 1;
                if (positioning->isFileSemiOpen(opposite, col) && positioning->countPawnsAtColumn(side, col) == 0)
                    backwardPawnsAtSemiopenFiles[side] -= 1;
            }
            updateAdjecentFilesCounter(side, col);
            int bottomRow = positioning->bottomPawns[col][side];
            if (adjecentFilesPropertyCount[col][side] == 2 && pathControlledByEnemyPawn[bottomRow][col][side] > 0)
            {
                updateBackwardCounters(side, opposite, col, 1);
                backwardPawnRows[col][side] = bottomRow;
            }
            else
                backwardPawnRows[col][side] = endValues[side];
        }
    }
    if (positioning->countPawnsAtColumn(side, col) == 0 && backwardPawnRows[col][(int)opposite] != endValues[(int)opposite])
        backwardPawnsAtSemiopenFiles[opposite] += 1;
}

void BackwardPawns::show() const
{
    std::cout << "Backward pawns in total (white | black): " << backwardPawns[0] << " | " << backwardPawns[1] << std::endl;
    std::cout << "Backward pawns at semi-open files (white | black): " << backwardPawnsAtSemiopenFiles[0] << " | " << backwardPawnsAtSemiopenFiles[1] << std::endl;
}





/// PassedPawns methods
void PassedPawns::clearTables()
{
    for (int i = 0; i < 8; i++)
    {
        passedConditionCounters[i][0] = passedConditionCounters[i][1] = 0;
        passersPoints[i][0][0] = passersPoints[i][0][1] = passersPoints[i][0][2] = 0;
        passersPoints[i][1][0] = passersPoints[i][1][1] = passersPoints[i][0][2] = 0;
        passedPawnRows[i][0] = passedPawnRows[i][1] = 0;
    }
    passedPawnPoints[0] = passedPawnPoints[1] = 0;
    passedPawnCounters[0] = passedPawnCounters[1] = 0;
}

void PassedPawns::updateSelfPoints(Side side, int row, int col, int factor, ParametrizationMode mode)
{
    int currentPoints = passersPoints[col][side][SELF_POINTS];
    passedPawnPoints[side] -= currentPoints;
    passersPoints[col][side][SELF_POINTS] = mode == MULTIPLICATE ? currentPoints * factor : currentPoints / factor;
    passedPawnPoints[side] += passersPoints[col][side][SELF_POINTS];
}

void PassedPawns::updatePairPoints(Side side, int row, int col, int factor, ParametrizationMode mode)
{
    int currentPoints = passersPoints[col][side][LEFT_SIDE];
    if (currentPoints > 0)
    {
        passedPawnPoints[side] -= currentPoints;
        passersPoints[col][side][LEFT_SIDE] = mode == MULTIPLICATE ? currentPoints * factor : currentPoints / factor;
        passersPoints[col - 1][side][RIGHT_SIDE] = passersPoints[col][side][LEFT_SIDE];
        passedPawnPoints[side] += passersPoints[col][side][LEFT_SIDE];
    }
    currentPoints = passersPoints[col][side][RIGHT_SIDE];
    if (currentPoints > 0)
    {
        passedPawnPoints[side] -= currentPoints;
        passersPoints[col][side][RIGHT_SIDE] = mode == MULTIPLICATE ? currentPoints * factor : currentPoints / factor;
        passersPoints[col + 1][side][LEFT_SIDE] = passersPoints[col][side][RIGHT_SIDE];
        passedPawnPoints[side] += passersPoints[col][side][RIGHT_SIDE];
    }
}

int PassedPawns::getPointsAfterPenalties(Side side, int row, int col)
{
    const Piece* piece = config->getPiece(row + frontSpawns[side], col);
    if (piece != nullptr && piece->getColor() != side && piece->getType() != PAWN)
        return positioning->passedPawnDistanceMultiplier[passedDistance[side](row)] / positioning->blockadeConnectedPassersMultipliers[piece->getType()];
    else
        return positioning->passedPawnDistanceMultiplier[passedDistance[side](row)];
}

void PassedPawns::addPassedPawn(Side side, int row, int col)
{
    int distancePoints = positioning->passedPawnDistanceMultiplier[passedDistance[side](row)];
    int selfPoints = distancePoints;
    // Protection --------
    if (control->countPawnAttacks(side, row, col) > 0)
        selfPoints *= positioning->protectedPasserMultiplier;
    // -------------------
    // Blockade ----------
    const Piece* piece = config->getPiece(row + frontSpawns[side], col);
    bool blockadeFlag = false;
    if (piece != nullptr && piece->getColor() != side && piece->getType() != PAWN)
    {
        blockadeFlag = true;
        selfPoints /= positioning->blockadeSinglePassersMultipliers[piece->getType()];
    }
    // -------------------
    passedPawnCounters[side] += 1;
    passersPoints[col][side][SELF_POINTS] = selfPoints;
    passedPawnPoints[side] += selfPoints;
    passedPawnRows[col][side] = row;
    if (col > 0)
    {
        int leftRow = passedPawnRows[col - 1][side];
        if (leftRow != 0)
        {
            int pairPoints = calculateConnectedPawnsPoints(distancePoints, getPointsAfterPenalties(side, leftRow, col - 1));
            pairPoints = blockadeFlag ? pairPoints / positioning->blockadeConnectedPassersMultipliers[piece->getType()] : pairPoints;
            passedPawnPoints[side] += pairPoints;
            passersPoints[col - 1][side][RIGHT_SIDE] = pairPoints;
            passersPoints[col][side][LEFT_SIDE] = pairPoints;
        }
    }
    if (col < 7)
    {
        int rightRow = passedPawnRows[col + 1][side];
        if (rightRow != 0)
        {
            int pairPoints = calculateConnectedPawnsPoints(distancePoints, getPointsAfterPenalties(side, rightRow, col + 1));
            pairPoints = blockadeFlag ? pairPoints / positioning->blockadeConnectedPassersMultipliers[piece->getType()] : pairPoints;
            passedPawnPoints[side] += pairPoints;
            passersPoints[col + 1][side][LEFT_SIDE] = pairPoints;
            passersPoints[col][side][RIGHT_SIDE] = pairPoints;
        }
    }
}

void PassedPawns::removePassedPawn(Side side, int row, int col)
{
    passedPawnCounters[side] -= 1;
    passedPawnPoints[side] -= passersPoints[col][side][SELF_POINTS];
    passersPoints[col][side][SELF_POINTS] = 0;
    passedPawnRows[col][side] = 0;
    int upperRow = positioning->upperPawns[col][side];
	if (passersPoints[col][side][LEFT_SIDE] > 0)
	{
        passedPawnPoints[side] -= passersPoints[col][side][LEFT_SIDE];
		passersPoints[col][side][LEFT_SIDE] = 0;
		passersPoints[col - 1][side][RIGHT_SIDE] = 0;
	}
	if (passersPoints[col][side][RIGHT_SIDE] > 0)
	{
        passedPawnPoints[side] -= passersPoints[col][side][RIGHT_SIDE];
		passersPoints[col][side][RIGHT_SIDE] = 0;
		passersPoints[col + 1][side][LEFT_SIDE] = 0;
	}
}

void PassedPawns::updateCountersWhenAddingPawn(Side side, Side opposite, int targetCol, int pawnRow, int pawnCol)
{
    if (isBottomRow[(int)opposite](positioning->upperPawns[targetCol][(int)opposite], pawnRow))
        passedConditionCounters[targetCol][(int)opposite] += 1;
    int passerRow = passedPawnRows[targetCol][(int)opposite];
    if (passerRow != 0 && passedConditionCounters[targetCol][(int)opposite] > 0)
        removePassedPawn(opposite, passerRow, targetCol);
}

// Inserting a pawn update
void PassedPawns::updateByInsertion(Side side, int row, int col)
{
    Side opposite = opposition[side];
    int conditionForCurrentCol = 0;
    updateCountersWhenAddingPawn(side, opposite, col, row, col);
    if (col > 0)
    {
        int leftCol = col - 1;
        updateCountersWhenAddingPawn(side, opposite, leftCol, row, col);
        conditionForCurrentCol += static_cast<int>(isBottomRow[side](row, positioning->bottomPawns[leftCol][(int)opposite]));
        if (passedPawnRows[leftCol][side] == row + frontSpawns[side] && passersPoints[leftCol][side][SELF_POINTS] % 3 != 0)
            updateSelfPoints(side, passedPawnRows[leftCol][side], leftCol, positioning->protectedPasserMultiplier, MULTIPLICATE);
    }
    if (col < 7)
    {
        int rightCol = col + 1;
        updateCountersWhenAddingPawn(side, opposite, rightCol, row, col);
        conditionForCurrentCol += static_cast<int>(isBottomRow[side](row, positioning->bottomPawns[rightCol][(int)opposite]));
        if (passedPawnRows[rightCol][side] == row + frontSpawns[side] && passersPoints[rightCol][side][SELF_POINTS] % 3 != 0)
            updateSelfPoints(side, passedPawnRows[rightCol][side], rightCol, positioning->protectedPasserMultiplier, MULTIPLICATE);
    }
    if (row == positioning->upperPawns[col][side])
    {
        if (passedPawnRows[col][side] != 0)
            removePassedPawn(side, passedPawnRows[col][side], col);
        conditionForCurrentCol += static_cast<int>(isBottomRow[side](row, positioning->bottomPawns[col][(int)opposite]));
        passedConditionCounters[col][side] = conditionForCurrentCol;
        if (conditionForCurrentCol == 0)
            addPassedPawn(side, row, col);
    }
}

void PassedPawns::updateCountersWhenRemovingPawn(Side side, Side opposite, int targetCol, int pawnRow, int pawnCol)
{
    if (isBottomRow[(int)opposite](positioning->upperPawns[targetCol][(int)opposite], pawnRow))
        passedConditionCounters[targetCol][(int)opposite] -= 1;
    int passerRow = passedPawnRows[targetCol][(int)opposite];
    int upperRow = positioning->upperPawns[targetCol][(int)opposite];
    if (passerRow == 0 && positioning->pawnsMap[upperRow][targetCol][(int)opposite] && passedConditionCounters[targetCol][(int)opposite] == 0)
        addPassedPawn(opposite, upperRow, targetCol);
}

// Removing a pawn update
void PassedPawns::updateByRemoval(Side side, int row, int col)
{
    Side opposite = opposition[side];
    int conditionForCurrentCol = 0;
    int upperRow = positioning->upperPawns[col][side];
    updateCountersWhenRemovingPawn(side, opposite, col, row, col);
    if (col > 0)
    {
        int leftCol = col - 1;
        updateCountersWhenRemovingPawn(side, opposite, leftCol, row, col);
        conditionForCurrentCol += static_cast<int>(isBottomRow[side](upperRow, positioning->bottomPawns[leftCol][(int)opposite]));
        if (passedPawnRows[leftCol][side] == row + frontSpawns[side] && passersPoints[leftCol][side][SELF_POINTS] % 3 == 0)
            updateSelfPoints(side, passedPawnRows[leftCol][side], leftCol, positioning->protectedPasserMultiplier, DIVIDE);
    }
    if (col < 7)
    {
        int rightCol = col + 1;
        updateCountersWhenRemovingPawn(side, opposite, rightCol, row, col);
        conditionForCurrentCol += static_cast<int>(isBottomRow[side](upperRow, positioning->bottomPawns[rightCol][(int)opposite]));
        if (passedPawnRows[rightCol][side] == row + frontSpawns[side] && passersPoints[rightCol][side][SELF_POINTS] % 3 == 0)
            updateSelfPoints(side, passedPawnRows[rightCol][side], rightCol, positioning->protectedPasserMultiplier, DIVIDE);
    }
    if (isBottomRow[side](upperRow, row))
    {
        if (passedPawnRows[col][side] != 0)
            removePassedPawn(side, row, col);
        conditionForCurrentCol += static_cast<int>(isBottomRow[side](upperRow, positioning->bottomPawns[col][(int)opposite]));
        passedConditionCounters[col][side] = conditionForCurrentCol;
        if (conditionForCurrentCol == 0 && positioning->pawnsMap[upperRow][col][side])
            addPassedPawn(side, upperRow, col);
    }
}

// Inserting or removing a blocker update
void PassedPawns::updateByPiecePositionChanged(Side side, PieceType type, int row, int col, ParametrizationMode mode)
{
    if (row >= blockerLowerBounds[side] && row <= blockerUpperBounds[side])
    {
        Side opposite = opposition[side];
        if (passedPawnRows[col][(int)opposite] == row + frontSpawns[side])
        {
            updateSelfPoints(opposite, passedPawnRows[col][(int)opposite], col, positioning->blockadeSinglePassersMultipliers[type], mode);
            updatePairPoints(opposite, passedPawnRows[col][(int)opposite], col, positioning->blockadeConnectedPassersMultipliers[type], mode);
        }
    }
}

void PassedPawns::show() const
{
    std::cout << "Passed pawns (white | black): " << passedPawnCounters[0] << " | " << passedPawnCounters[1] << std::endl;
    std::cout << "Passed pawns points (white | black): " << passedPawnPoints[0] << " | " << passedPawnPoints[1] << std::endl;
}





/// Evaluation
void PawnStructure::initTables()
{
    for (int i = 0; i < 33; i++)
    {
        float w = (32.f - i) / 32.f;
        isolatedPawnPointValue[i] = interpolation(factors[ISOLATED_PAWNS], w, [](float x)->float {return x; });
        doubledPawnValue[i] = interpolation(factors[DOUBLED_PAWNS], w, [](float x)->float {return x; });
        pawnIslandValue[i] = interpolation(factors[PAWN_ISLANDS], w, [](float x)->float {return x; });
        backwardPawnValue[i] = interpolation(factors[BACKWARD_PAWN_PENALTY], w, [](float x)->float {return x; });
    }
    for (int i = 6; i > 0; i--)
    {
        passedPawnDistanceMultiplier[i] = interpolation(factors[PASSED_PAWN_DISTANCE_MULTIPLIER], (6.f - i) / 5.f,
            [](float x)->float {return std::pow(2.f, 5.f * x - 5.f); });
    }
    backwardPawnOpenFilePenalty = factors[BACKWARD_PAWN_OPEN_FILE_PENALTY].startValue;
    passedPawnBaseValue = factors[PASSED_PAWN_BASE_VALUE].startValue;
    protectedPasserMultiplier = factors[PROTECTED_PASSER_MULTIPLIER].startValue;
    passedPawnPointValue = factors[PASSED_PAWN_POINT_VALUE].startValue;
}

void IsolatedPawns::evaluate(int& eval, const int& gameStage) const
{
    int sum = (isolatedPawns[0] - isolatedPawns[1]) * positioning->isolatedPawnPointValue[gameStage];
    eval += sum;
    std::cout << "Isolated pawns evaluation: " << sum << std::endl;
}

void DoubledPawns::evaluate(int& eval, const int& gameStage) const
{
    int sum = (doubledPawns[0] - doubledPawns[1]) * positioning->doubledPawnValue[gameStage];
    eval += sum;
    std::cout << "Doubled pawns evaluation: " << sum << std::endl;

}

void PawnIslands::evaluate(int& eval, const int& gameStage) const
{
    int sum = (pawnIslands[0] - pawnIslands[1]) * positioning->pawnIslandValue[gameStage];
    eval += sum;
    std::cout << "Pawn islands evaluation: " << sum << std::endl;
}

void BackwardPawns::evaluate(int& eval, const int& gameStage) const
{
    int sum = (backwardPawns[0] - backwardPawns[1]) * positioning->backwardPawnValue[gameStage];
    sum += backwardPawnsAtSemiopenFiles[0] * positioning->backwardPawnOpenFilePenalty * positioning->material->countRooks(BLACK);
    sum -= backwardPawnsAtSemiopenFiles[1] * positioning->backwardPawnOpenFilePenalty * positioning->material->countRooks(WHITE);
    eval += sum;
    std::cout << "Backward pawns evaluation: " << sum << std::endl;
}

void PassedPawns::evaluate(int& eval, const int& gameStage) const
{
    int sum = 0;
    sum += (passedPawnCounters[0] - passedPawnCounters[1]) * positioning->passedPawnBaseValue;
    sum += (passedPawnPoints[0] - passedPawnPoints[1]) * positioning->passedPawnPointValue;
    eval += sum;
    std::cout << "Passed pawns evaluation: " << sum << std::endl;
}

int PawnStructure::evaluate(int& eval, const int& gameStage) const
{
    int startValue = eval;
    for (PawnsWatcher* watcher : columnWatchers)
        watcher->evaluate(eval, gameStage);
    for (PawnsWatcher* watcher : generalWatchers)
        watcher->evaluate(eval, gameStage);
    return eval - startValue;
}
