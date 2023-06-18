#include "pawnstructure.h"
// Remember about negative factor!

PawnStructure::PawnStructure(BoardConfig* cnf, MaterialBalance* mt) : PositionElement("PawnStructure"), config(cnf), material(mt)
{
    IsolatedPawns* isolated = new IsolatedPawns(this);
    DoubledPawns* doubled = new DoubledPawns(this);
    PawnIslands* islands = new PawnIslands(this);
    BackwardPawns* backwards = new BackwardPawns(this);
    pawnsWatchers.push_back(isolated);
    pawnsWatchers.push_back(doubled);
    pawnsWatchers.push_back(islands);
    pawnsWatchersAdvanced.push_back(backwards);
}

PawnStructure::~PawnStructure()
{
    for (unsigned int i = 0; i < pawnsWatchers.size(); i++)
        delete pawnsWatchers[i];
}

void PawnStructure::clearTables()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            tablePawnsAtColumn[i][j] = 0;
            tableBottomPawns[i][0] = 0;
            tableBottomPawns[i][1] = 7;
            for (int k = 0; k < 8; k++)
                pawnsMap[i][k][j] = false;
        }
    }
    topLeftPawns[0] = topLeftPawns[1] = 7;
    topRightPawns[0] = topRightPawns[1] = 0;
    tablePawnsAtColor[0][0] = tablePawnsAtColor[0][1] = 0;
    tablePawnsAtColor[1][0] = tablePawnsAtColor[1][1] = 0;
    tableSemiOpenFiles[0] = tableSemiOpenFiles[1] = 8;
}

void PawnStructure::resetSubelements()
{
    for (PawnsWatcher* element : pawnsWatchers)
        element->reset();
    for (PawnsWatcherExtended* element : pawnsWatchersAdvanced)
        element->reset();
}

void PawnStructure::addPawn(int col, int row, int colorID, bool columnUpdateFlag)
{
    pawnsMap[row][col][colorID] = true;
    tablePawnsAtColor[(row + col) % 2][colorID] += 1;
    if (columnUpdateFlag)
    {
        tablePawnsAtColumn[col][colorID] += 1;
        if (tablePawnsAtColumn[col][colorID] == 1)
            tableSemiOpenFiles[colorID] -= 1;
    }
    if (colorID == 0 && row > tableBottomPawns[col][0])
        tableBottomPawns[col][0] = row;
    else if (colorID == 1 && row < tableBottomPawns[col][1])
        tableBottomPawns[col][1] = row;
    if (col < topLeftPawns[colorID])
        topLeftPawns[colorID] = col;
    if (col > topRightPawns[colorID])
        topRightPawns[colorID] = col;
}

void PawnStructure::removePawn(int col, int row, int colorID, bool columnUpdateFlag)
{
    pawnsMap[row][col][colorID] = false;
    tablePawnsAtColor[(row + col) % 2][colorID] -= 1;
    if (columnUpdateFlag)
    {
        tablePawnsAtColumn[col][colorID] -= 1;
        if (tablePawnsAtColumn[col][colorID] == 0)
            tableSemiOpenFiles[colorID] += 1;
    }
    if (row == tableBottomPawns[col][colorID])
    {
        if (colorID == 0)
        {
            tableBottomPawns[col][0] = 0;
            for (int i = row - 1; i > 1; i--)
            {
                if (pawnsMap[i][col][0])
                {
                    tableBottomPawns[col][0] = i;
                    break;
                }
            }
        }
        else
        {
            tableBottomPawns[col][1] = 7;
            for (int i = row + 1; i < 6; i++)
            {
                if (pawnsMap[i][col][1])
                {
                    tableBottomPawns[col][1] = i;
                    break;
                }
            }
        }
    }
    if (topLeftPawns[colorID] == col && tablePawnsAtColumn[col][colorID] == 0)
    {
        if (topRightPawns[colorID] == topLeftPawns[colorID])
            topRightPawns[colorID] = 0;
        topLeftPawns[colorID] = 7;
        for (int i = col + 1; i < 7; i++)
        {
            if (tablePawnsAtColumn[i][colorID] > 0)
            {
                topLeftPawns[colorID] = i;
                break;
            }
        }
    }
    else if (topRightPawns[colorID] == col && tablePawnsAtColumn[col][colorID] == 0)
    {
        topRightPawns[colorID] = 0;
        for (int i = col - 1; i > 0; i--)
        {
            if (tablePawnsAtColumn[i][colorID] > 0)
            {
                topRightPawns[colorID] = i;
                break;
            }
        }
    }
}

void PawnStructure::reset()
{
    clearTables();
    resetSubelements();
    for (int i = 0; i < 32; i++)
    {
        Piece* piece = config->getPiece(i);
        if (piece->isActive() && BoardConfig::isPawn(piece))
        {
            const sf::Vector2i& pos = piece->getPos();
            COLOR color = piece->getColor();
            addPawn(pos.x, pos.y, (int)color);
            updateElementsByInsertion(pos.x, color);
            updateElementsByInsertion(pos.x, pos.y, color);
        }

    }
}

void PawnStructure::updateElementsByInsertion(int col, COLOR color)
{
    for (PawnsWatcher* element : pawnsWatchers)
        element->updateByInsertion(col, color);
}

void PawnStructure::updateElementsByInsertion(int col, int row, COLOR color)
{
    for (PawnsWatcherExtended* element : pawnsWatchersAdvanced)
        element->updateByInsertion(col, row, color);
}

void PawnStructure::updateElementsByRemoval(int col, COLOR color)
{
    for (PawnsWatcher* element : pawnsWatchers)
        element->updateByRemoval(col, color);
}

void PawnStructure::updateElementsByRemoval(int col, int row, COLOR color)
{
    for (PawnsWatcherExtended* element : pawnsWatchersAdvanced)
        element->updateByRemoval(col, row, color);
}

void PawnStructure::updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
{
    if (oldPos.x < 8 && newPos.x < 8 && BoardConfig::isPawn(config->getPiece(pieceID)))
    {
        COLOR color = config->getPiece(pieceID)->getColor();
        if (oldPos.x != newPos.x)
        {
            removePawn(oldPos.x, oldPos.y, (int)color, true);
            updateElementsByRemoval(oldPos.x, color);
            updateElementsByRemoval(oldPos.x, oldPos.y, color);
            addPawn(newPos.x, newPos.y, (int)color, true);
            updateElementsByInsertion(newPos.x, color);
            updateElementsByInsertion(newPos.x, newPos.y, color);
        }
        else
        {
            removePawn(oldPos.x, oldPos.y, (int)color, false);
            updateElementsByRemoval(oldPos.x, oldPos.y, color);
            addPawn(newPos.x, newPos.y, (int)color, false);
            updateElementsByInsertion(newPos.x, newPos.y, color);
        }
    }
}

void PawnStructure::updateByChange(int pieceID, const sf::Vector2i& oldPos, PieceType oldType, PieceType newType)
{
    Piece* piece = config->getPiece(pieceID);
    COLOR color = piece->getColor();
    if (oldType == PieceType::PAWN)
    {
        removePawn(oldPos.x, oldPos.y, (int)color);
        updateElementsByRemoval(oldPos.x, color);
        updateElementsByRemoval(oldPos.x, oldPos.y, color);
    }
    else if (newType == PieceType::PAWN)
    {
        const sf::Vector2i& pos = piece->getPos();
        addPawn(pos.x, pos.y, (int)color);
        updateElementsByInsertion(pos.x, color);
        updateElementsByInsertion(pos.x, pos.y, color);
    }
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
    std::cout<<"White pawns on light squares: "<<tablePawnsAtColor[0][0]<<std::endl;
    std::cout<<"White pawns on dark squares: "<<tablePawnsAtColor[1][0]<<std::endl;
    std::cout<<"Black pawns on light squares: "<<tablePawnsAtColor[0][1]<<std::endl;
    std::cout<<"Black pawns on dark squares: "<<tablePawnsAtColor[1][1]<<std::endl;
    std::cout<<"White bottom pawns: ";
    for (int i = 0; i < 8; i++)
        std::cout<<tableBottomPawns[i][0]<<" ";
    std::cout<<std::endl;
    std::cout<<"Black bottom pawns: ";
    for (int i = 0; i < 8; i++)
        std::cout<<tableBottomPawns[i][1]<<" ";
    std::cout<<std::endl;
    std::cout<<"White left & right pawns: "<<topLeftPawns[0]<<" "<<topRightPawns[0]<<std::endl;
    std::cout<<"Black left & right pawns: "<<topLeftPawns[1]<<" "<<topRightPawns[1]<<std::endl;
    for (PawnsWatcher* element : pawnsWatchers)
        element->show();
    for (PawnsWatcherExtended* element : pawnsWatchersAdvanced)
        element->show();
}




void IsolatedPawns::clearTables()
{
    value = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
            columnsIsolation[i][j] = 0;
    }
}

void IsolatedPawns::updateByInsertion(int col, COLOR color)
{
    int colorID = (int)color;
    if (positioning->pawnsAt(col, colorID) == 1)
    {
        if (col > 0)
        {
            if (columnsIsolation[col - 1][colorID] == 0)
            {
                if (color == COLOR::WHITE)
                    value -= positioning->pawnsAt(col - 1, colorID);
                else
                    value += positioning->pawnsAt(col - 1, colorID);
            }
            columnsIsolation[col - 1][colorID] += 1;
        }
        if (col < 7)
        {
            if (columnsIsolation[col + 1][colorID] == 0)
            {
                if (color == COLOR::WHITE)
                    value -= positioning->pawnsAt(col + 1, colorID);
                else
                    value += positioning->pawnsAt(col + 1, colorID);
            }
            columnsIsolation[col + 1][colorID] += 1;
        }
    }
    if (columnsIsolation[col][colorID] == 0)
    {
        if (color == COLOR::WHITE)
            value += 1;
        else
            value -= 1;
    }
}

void IsolatedPawns::updateByRemoval(int col, COLOR color)
{
    int colorID = (int)color;
    if (positioning->pawnsAt(col, colorID) == 0)
    {
        if (col > 0)
        {
            columnsIsolation[col - 1][colorID] -= 1;
            if (columnsIsolation[col - 1][colorID] == 0)
            {
                if (color == COLOR::WHITE)
                    value += positioning->pawnsAt(col - 1, colorID);
                else
                    value -= positioning->pawnsAt(col - 1, colorID);
            }
        }
        if (col < 7)
        {
            columnsIsolation[col + 1][colorID] -= 1;
            if (columnsIsolation[col + 1][colorID] == 0)
            {
                if (color == COLOR::WHITE)
                    value += positioning->pawnsAt(col + 1, colorID);
                else
                    value -= positioning->pawnsAt(col + 1, colorID);
            }
        }
    }
    if (columnsIsolation[col][colorID] == 0)
    {
        if (color == COLOR::WHITE)
            value -= 1;
        else
            value += 1;
    }
}




void DoubledPawns::updateByInsertion(int col, COLOR color)
{
    int pawnsCount = positioning->pawnsAt(col, (int)color);
    if (color == COLOR::WHITE)
    {
        value -= numberOfPair(pawnsCount - 1);
        value += numberOfPair(pawnsCount);
    }
    else
    {
        value += numberOfPair(pawnsCount - 1);
        value -= numberOfPair(pawnsCount);
    }
}

void DoubledPawns::updateByRemoval(int col, COLOR color)
{
    int pawnsCount = positioning->pawnsAt(col, (int)color);
    if (color == COLOR::WHITE)
    {
        value -= numberOfPair(pawnsCount + 1);
        value += numberOfPair(pawnsCount);
    }
    else
    {
        value += numberOfPair(pawnsCount + 1);
        value -= numberOfPair(pawnsCount);
    }
}




void PawnIslands::updateByInsertion(int col, COLOR color)
{
    int colorID = (int)color;
    if (positioning->pawnsAt(col, colorID) == 1)
    {
        bool leftFlag = false;
        bool rightFlag = false;
        if (col > 0 && positioning->pawnsAt(col - 1, colorID) > 0)
            leftFlag = true;
        if (col < 7 && positioning->pawnsAt(col + 1, colorID) > 0)
            rightFlag = true;
        if (leftFlag && rightFlag)
        {
            if (color == COLOR::WHITE)
                value -= 1;
            else
                value += 1;
        }
        else if (!leftFlag && !rightFlag)
        {
            if (color == COLOR::WHITE)
                value += 1;
            else
                value -= 1;
        }
    }
}

void PawnIslands::updateByRemoval(int col, COLOR color)
{
    int colorID = (int)color;
    if (positioning->pawnsAt(col, colorID) == 0)
    {
        bool leftFlag = false;
        bool rightFlag = false;
        if (col > 0 && positioning->pawnsAt(col - 1, colorID) > 0)
            leftFlag = true;
        if (col < 7 && positioning->pawnsAt(col + 1, colorID) > 0)
            rightFlag = true;
        if (leftFlag & rightFlag)
        {
            if (color == COLOR::WHITE)
                value += 1;
            else
                value -= 1;
        }
        else if (!leftFlag && !rightFlag)
        {
            if (color == COLOR::WHITE)
                value -= 1;
            else
                value += 1;
        }
    }
}




void BackwardPawns::clearTables()
{
    value = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                sentinels[i][j][k] = 0;
                bottomPawnFlags[i][j][k] = false;
            }
        }
    }
}

void BackwardPawns::updateByInsertion(int col, int row, COLOR color)
{
    if (color == COLOR::WHITE)
    {
        if (row < 3)
            return;
        bool bottomUpdateFlag = row == positioning->bottomPawnFromColumn(col, 0);
        bool leftFlag = false;
        bool rightFlag = false;
        if (col > 0)
        {
            sentinels[row - 2][col - 1][1] += 1;
            if (sentinels[row - 2][col - 1][1] == 1 && bottomPawnFlags[row - 2][col - 1][1])
                value -= 1;
            if (bottomUpdateFlag)
            {
                int leftRow = positioning->bottomPawnFromColumn(col - 1, 0);
                if (leftRow < row)
                    leftFlag = true;
                if (bottomPawnFlags[leftRow][col - 1][0] && leftRow <= row)
                {
                    if (sentinels[leftRow][col - 1][0] > 0)
                        value -= 1;
                    bottomPawnFlags[leftRow][col - 1][0] = false;
                }
            }
        }
        if (col < 7)
        {
            sentinels[row - 2][col + 1][1] += 1;
            if (sentinels[row - 2][col + 1][1] == 1 && bottomPawnFlags[row - 2][col + 1][1])
                value -= 1;
            if (bottomUpdateFlag)
            {
                int rightRow = positioning->bottomPawnFromColumn(col + 1, 0);
                if (rightRow < row)
                    rightFlag = true;
                if (bottomPawnFlags[rightRow][col + 1][0] && rightRow <= row)
                {
                    if (sentinels[rightRow][col + 1][0] > 0)
                        value -= 1;
                    bottomPawnFlags[rightRow][col + 1][0] = false;
                }
            }
        }
        if (leftFlag && rightFlag)
        {
            bottomPawnFlags[row][col][0] = true;
            if (sentinels[row][col][0] > 0)
                value += 1;
        }
    }
    else
    {
        if (row > 4)
            return;
        bool bottomUpdateFlag = row == positioning->bottomPawnFromColumn(col, 1);
        bool leftFlag = false;
        bool rightFlag = false;
        if (col > 0)
        {
            sentinels[row + 2][col - 1][0] += 1;
            if (sentinels[row + 2][col - 1][0] == 1 && bottomPawnFlags[row + 2][col - 1][0])
                value += 1;
            if (bottomUpdateFlag)
            {
                int leftRow = positioning->bottomPawnFromColumn(col - 1, 1);
                if (leftRow > row)
                    leftFlag = true;
                if (bottomPawnFlags[leftRow][col - 1][1] && leftRow >= row)
                {
                    if (sentinels[leftRow][col - 1][1] > 0)
                        value += 1;
                    bottomPawnFlags[leftRow][col - 1][1] = false;
                }
            }
        }
        if (col < 7)
        {
            sentinels[row + 2][col + 1][0] += 1;
            if (sentinels[row + 2][col + 1][0] == 1 && bottomPawnFlags[row + 2][col + 1][0])
                value += 1;
            if (bottomUpdateFlag)
            {
                int rightRow = positioning->bottomPawnFromColumn(col + 1, 1);
                if (rightRow > row)
                    rightFlag = true;
                if (bottomPawnFlags[rightRow][col + 1][1] && rightRow >= row)
                {
                    if (sentinels[rightRow][col + 1][1] > 0)
                        value += 1;
                    bottomPawnFlags[rightRow][col + 1][1] = false;
                }
            }
        }
        if (leftFlag && rightFlag)
        {
            bottomPawnFlags[row][col][1] = true;
            if (sentinels[row][col][1] > 0)
                value -= 1;
        }
    }
}

void BackwardPawns::updateByRemoval(int col, int row, COLOR color)
{
    if (color == COLOR::WHITE)
    {
        if (row < 3)
            return;
        bool bottomUpdateFlag = row > positioning->bottomPawnFromColumn(col, 0);
        if (col > 0)
        {
            sentinels[row - 2][col - 1][1] -= 1;
            if (sentinels[row - 2][col - 1][1] == 0 && bottomPawnFlags[row - 2][col - 1][1])
                value += 1;
            if (bottomUpdateFlag)
            {
                int leftRow = positioning->bottomPawnFromColumn(col - 1, 0);
                if (!bottomPawnFlags[leftRow][col - 1][0] && leftRow > positioning->bottomPawnFromColumn(col, 0)  &&
                    (col - 1 == 0 || leftRow > positioning->bottomPawnFromColumn(col - 2, 0)))
                {
                    if (sentinels[leftRow][col - 1][0] > 0)
                        value += 1;
                    bottomPawnFlags[leftRow][col - 1][0] = true;
                }
            }
        }
        if (col < 7)
        {
            sentinels[row - 2][col + 1][1] -= 1;
            if (sentinels[row - 2][col + 1][1] == 0 && bottomPawnFlags[row - 2][col + 1][1])
                value += 1;
            if (bottomUpdateFlag)
            {
                int rightRow = positioning->bottomPawnFromColumn(col + 1, 0);
                if (!bottomPawnFlags[rightRow][col + 1][0] && rightRow > positioning->bottomPawnFromColumn(col, 0) &&
                    (col + 1 == 7 || rightRow > positioning->bottomPawnFromColumn(col + 2, 0)))
                {
                    if (sentinels[rightRow][col + 1][0] > 0)
                        value += 1;
                    bottomPawnFlags[rightRow][col + 1][0] = true;
                }
            }
        }
        if (isBackwards(col, row, 0))
            value -= 1;
        bottomPawnFlags[row][col][0] = false;
    }
    else
    {
        if (row > 4)
            return;
        bool bottomUpdateFlag = row < positioning->bottomPawnFromColumn(col, 1);
        if (col > 0)
        {
            sentinels[row + 2][col - 1][0] -= 1;
            if (sentinels[row + 2][col - 1][0] == 0 && bottomPawnFlags[row + 2][col - 1][0])
                value -= 1;
            if (bottomUpdateFlag)
            {
                int leftRow = positioning->bottomPawnFromColumn(col - 1, 1);
                if (!bottomPawnFlags[leftRow][col - 1][1] && leftRow < positioning->bottomPawnFromColumn(col, 1) &&
                    (col - 1 == 0 || leftRow < positioning->bottomPawnFromColumn(col - 2, 1)))
                {
                    if (sentinels[leftRow][col - 1][1] > 0)
                        value -= 1;
                    bottomPawnFlags[leftRow][col - 1][1] = true;
                }
            }
        }
        if (col < 7)
        {
            sentinels[row + 2][col + 1][0] -= 1;
            if (sentinels[row + 2][col + 1][0] == 0 && bottomPawnFlags[row + 2][col + 1][0])
                value -= 1;
            if (bottomUpdateFlag)
            {
                int rightRow = positioning->bottomPawnFromColumn(col + 1, 1);
                if (!bottomPawnFlags[rightRow][col + 1][1] && rightRow < positioning->bottomPawnFromColumn(col, 1) &&
                    (col + 1 == 7 || rightRow < positioning->bottomPawnFromColumn(col + 2, 1)))
                {
                    if (sentinels[rightRow][col + 1][1] > 0)
                        value -= 1;
                    bottomPawnFlags[rightRow][col + 1][1] = true;
                }
            }
        }
        if (isBackwards(col, row, 1))
            value += 1;
        bottomPawnFlags[row][col][1] = false;
    }
}

