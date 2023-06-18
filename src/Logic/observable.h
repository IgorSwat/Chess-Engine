#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include "move.h"
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
    bool isReseted() const {return resetState;}
    void setState(bool state) {resetState = state;}
    virtual void reset() = 0;
};

class PositionChangedObserver : virtual public ObserverBase
{
public:
    virtual ~PositionChangedObserver() {}
    virtual void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos) = 0;
};

class MoveListChangedObserver : virtual public ObserverBase
{
public:
    virtual ~MoveListChangedObserver() {}
    virtual void updateByRemoval(const vector<Move2>& moves) = 0;
    virtual void updateByInsertion(const Move2& move) = 0;
};

class PieceTypeChangedObserver : virtual public ObserverBase
{
public:
    virtual ~PieceTypeChangedObserver() {}
    virtual void updateByChange(int pieceID, const sf::Vector2i& oldPos, PieceType oldType, PieceType newType) = 0;
};


class PawnControlChangedObserver : virtual public ObserverBase
{
public:
    virtual ~PawnControlChangedObserver() {}
    virtual void updateByAppearance(const sf::Vector2i& pos, int side) = 0;
    virtual void updateByDisappearance(const sf::Vector2i& pos, int side) = 0;
};





template <typename ObserverType>
class Observable
{
protected:
    vector<ObserverType*> observers;
public:
    Observable() {}
    void addObserver(ObserverType* observer) {observers.push_back(observer);}
    void removeObserver(ObserverType* observer);
    void resetObservers();
};

template <typename ObserverType>
void Observable<ObserverType>::removeObserver(ObserverType* observer)
{
    auto it = std::remove_if(observers.begin(), observers.end(),
                                                [&observer](ObserverType* o)->bool{return o == observer;});
    observers.erase(it, observers.end());
}

template <typename ObserverType>
void Observable<ObserverType>::resetObservers()
{
    for (ObserverType* observer : observers)
    {
        observer->setState(false);
    }
    for (unsigned int i = 0; i < observers.size(); i++)
    {
        ObserverBase* observer = observers[i];
        if (!observer->isReseted())
        {
            observer->setState(true);
            observer->reset();
        }
    }
}




class ObservablePosition : public Observable<PositionChangedObserver>
{
public:
    void updateObserversByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
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
    void updateObserversByRemoval(const std::vector<Move2>& movesList)
    {
        for (MoveListChangedObserver* observer : observers)
            observer->updateByRemoval(movesList);
    }
};

class ObservableMaterial : public Observable<PieceTypeChangedObserver>
{
public:
    void updateObserversByChange(int pieceID, const sf::Vector2i& oldPos, PieceType oldType, PieceType newType)
    {
        for (PieceTypeChangedObserver* observer : observers)
            observer->updateByChange(pieceID, oldPos, oldType, newType);
    }
};

class ObservablePawnControl : public Observable<PawnControlChangedObserver>
{
public:
    void updateObserversByAppearance(const sf::Vector2i& pos, int side)
    {
        for (PawnControlChangedObserver* observer : observers)
            observer->updateByAppearance(pos, side);
    }
    void updateObserversByDisappearance(const sf::Vector2i& pos, int side)
    {
        for (PawnControlChangedObserver* observer : observers)
            observer->updateByDisappearance(pos, side);
    }
};

#endif // OBSERVABLE_H
