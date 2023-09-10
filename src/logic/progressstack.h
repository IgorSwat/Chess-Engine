#pragma once

#include <stack>
#include <vector>
#include "configchange.h"

using ChangesVec = std::vector<ConfigChange*>;

class ProgressStack
{
private:
    std::stack<ChangesVec*> boardProgress;
    ChangesVec currentLoader;
public:
    ProgressStack() {};
    bool isEmpty() const {return boardProgress.size() == 0;}
    void registerChange(ConfigChange* change) {currentLoader.push_back(change);}
    void pushChanges()
    {
        boardProgress.push(new ChangesVec(currentLoader));
        currentLoader.clear();
    }
    ChangesVec* getLatestChanges()
    {
        ChangesVec* changes = boardProgress.top();
        boardProgress.pop();
        return changes;
    }
};