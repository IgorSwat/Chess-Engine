#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include "move.h"
#include "movelist.h"
#include <vector>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using std::vector;

class ObserverBase
{
private:
    bool resetState = false;
public:
    virtual ~ObserverBase() {}
    bool isReseted() const { return resetState; }
    void setState(bool state) { resetState = state; }
};

class PositionChangedObserver : virtual public ObserverBase
{
public:
    virtual ~PositionChangedObserver() = default;
    virtual void updateByMove(int pieceID, const Square& oldPos, const Square& newPos) = 0;
};

class MoveListChangedObserver : virtual public ObserverBase
{
public:
    virtual ~MoveListChangedObserver() = default;
    virtual void updateByRemoval(int pieceID, const vector<Move2>& moves, bool legal) = 0;
    virtual void updateByRemoval(int pieceID, const MoveList& moves, bool legal) = 0;
    virtual void updateByInsertion(const Move2& move) = 0;
};

class PieceTypeChangedObserver : virtual public ObserverBase
{
public:
    virtual ~PieceTypeChangedObserver() = default;
    virtual void updateByChange(int pieceID, const Square& oldPos, PieceType oldType, PieceType newType) = 0;
};


class PawnControlChangedObserver : virtual public ObserverBase
{
public:
    virtual ~PawnControlChangedObserver() {}
    virtual void updateByAppearance(const Square& pos, Side side) = 0;
    virtual void updateByDisappearance(const Square& pos, Side side) = 0;
};


class EngineObserver : public ObserverBase
{
public:
    virtual ~EngineObserver() = default;
    virtual void reloadEngine() = 0;
};



template <typename ObserverType>
class Observable
{
protected:
    vector<ObserverType*> observers;
public:
    Observable() {}
    void addObserver(ObserverType* observer) { observers.push_back(observer); }
    void removeObserver(ObserverType* observer);
};

template <typename ObserverType>
void Observable<ObserverType>::removeObserver(ObserverType* observer)
{
    auto it = std::remove_if(observers.begin(), observers.end(),
        [&observer](ObserverType* o)->bool {return o == observer; });
    observers.erase(it, observers.end());
}




class ObservablePosition : public Observable<PositionChangedObserver>
{
public:
    void updateObserversByMove(int pieceID, const Square& oldPos, const Square& newPos)
    {
        for (PositionChangedObserver* observer : observers)
            observer->updateByMove(pieceID, oldPos, newPos);
    }
};


class ObservableMoves : public Observable<MoveListChangedObserver>
{
public:
    void updateObserversByInsertion(const Move2& move)
    {
        for (MoveListChangedObserver* observer : observers)
            observer->updateByInsertion(move);
    }
    void updateObserversByRemoval(int pieceID, const std::vector<Move2>& movesList, bool legal = true)
    {
        for (MoveListChangedObserver* observer : observers)
            observer->updateByRemoval(pieceID, movesList, legal);
    }
    void updateObserversByRemoval(int pieceID, const MoveList& movesList, bool legal = true)
    {
        for (MoveListChangedObserver* observer : observers)
            observer->updateByRemoval(pieceID, movesList, legal);
    }
};


class ObservableMaterial : public Observable<PieceTypeChangedObserver>
{
public:
    void updateObserversByChange(int pieceID, const Square& oldPos, PieceType oldType, PieceType newType)
    {
        for (PieceTypeChangedObserver* observer : observers)
            observer->updateByChange(pieceID, oldPos, oldType, newType);
    }
};


class ObservablePawnControl : public Observable<PawnControlChangedObserver>
{
public:
    void updateObserversByAppearance(const Square& pos, Side side)
    {
        for (PawnControlChangedObserver* observer : observers)
            observer->updateByAppearance(pos, side);
    }
    void updateObserversByDisappearance(const Square& pos, Side side)
    {
        for (PawnControlChangedObserver* observer : observers)
            observer->updateByDisappearance(pos, side);
    }
};


#endif // OBSERVABLE_H