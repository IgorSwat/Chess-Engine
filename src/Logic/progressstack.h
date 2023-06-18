#ifndef PROGRESSSTACK_H
#define PROGRESSSTACK_H

#include <stack>
#include <vector>
#include "configchange.h"
using std::stack;
using std::vector;

class ProgressStack
{
private:
    stack< vector<ConfigChange*>* > boardProgress;
    vector<ConfigChange*> currentLoader;
public:
    ProgressStack() {};
    bool isEmpty() const {return boardProgress.size() == 0;}
    void registerChange(ConfigChange* change) {currentLoader.push_back(change);}
    void pushChanges()
    {
        boardProgress.push(new vector<ConfigChange*>(currentLoader));
        currentLoader.clear();
    }
    vector<ConfigChange*>* getLatestChanges()
    {
        vector<ConfigChange*>* changes = boardProgress.top();
        boardProgress.pop();
        return changes;
    }
};

#endif // PROGRESSSTACK_H
