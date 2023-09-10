#ifndef POSITIONELEMENT_H
#define POSITIONELEMENT_H

#include "factorsdelivery.h"
#include "../logic/movegenerator.h"
#include <string>
#include <algorithm>
#include <exception>

class PositionElement
{
private:
    const std::string name;
    int factorsNum;
protected:
    FactorsVec factors;
public:
    PositionElement(std::string elName, const FactorsVec& factors, int factorsNum) : name(elName), factorsNum(factorsNum), factors(factors)
    {
        if (factorsNum > factors.size())
        {
            std::string error = "Too few factors provided for instance of " + name + "\n";
            throw std::invalid_argument(error);
        }
    }
    virtual ~PositionElement() {}
    const std::string& getName() const {return name;}
    virtual void update() = 0;
    virtual int evaluate(int& eval, const int& gameStage) const = 0;
    virtual void show() const {std::cout<<name<<std::endl;}
};


#endif // POSITIONELEMENT_H
