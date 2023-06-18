#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include "piece.h"

inline int mapPieceType(PieceType type) {return (int)type - 1;}

inline int mapPieceTypeByValue(PieceType type)
{
    int typeID = (int)type;
    if (typeID > 1)
        return typeID - 1;
    else
        return typeID;
}


#endif // MISC_H_INCLUDED
