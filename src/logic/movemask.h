#ifndef MOVEMASK_H
#define MOVEMASK_H

#include <iostream>
enum class MoveType {COMMON = 0, ATTACK, CASTLE, ENPASSANT, NONE};

class MoveMask
{
private:
    static constexpr int maskSize = 4;
    bool mask[maskSize];
public:
    MoveMask(MoveType mt)
    {
        for (int i = 0; i < maskSize; i++)
            mask[i] = false;
        mask[(int)mt] = true;
    }
    MoveMask& operator|(MoveType mt)
    {
        mask[(int)mt] = true;
        return (*this);
    }
    MoveMask& operator|(MoveMask&& other)
    {
        for (int i = 0; i < maskSize; i++)
            mask[i] = mask[i] || other.mask[i];
        return (*this);
    }
    bool isCommon() const {return mask[0];}
    bool isAttacking() const {return mask[1];}
    MoveMask getLastFlag() const
    {
        for (int i = maskSize - 1; i > 0; i++) {if (mask[i]) return (MoveType)i;}
        return MoveType::COMMON;
    }
    void show() const
    {
        for (int i = 0; i < maskSize; i++)
            std::cout<<mask[i]<<" ";
    }
};

inline MoveMask operator|(MoveType t1, MoveType t2) {return MoveMask(t1) | t2;}

#endif // MOVEMASK_H
