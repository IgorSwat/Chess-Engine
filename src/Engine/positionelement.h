#ifndef POSITIONELEMENT_H
#define POSITIONELEMENT_H

#include "movegenerator.h"
#include <string>
#include <algorithm>
using std::string;

class PositionElement
{
protected:
    const string name;
public:
    PositionElement(const string& elName) : name(elName) {}
    virtual ~PositionElement() {}
    string getName() const {return name;}
    virtual void evaluate() = 0;
    virtual void show() const {std::cout<<name<<std::endl;}
};


#endif // POSITIONELEMENT_H
