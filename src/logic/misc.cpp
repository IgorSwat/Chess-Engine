#include "misc.h"
#include <cmath>

int mapPieceTypeByValue(PieceType type)
{
    int typeID = (int)type;
    if (typeID > 1)
        return typeID - 1;
    else
        return typeID;
}

int interpolation(const int& begVal, const int& endVal, const float& w, float (*func)(float))
{
    float pw = func(w);
    return static_cast<int>(begVal * (1.f - pw) + endVal * pw);
}

int interpolation(const Factor& vals, const float& w, float (*func)(float)) { return interpolation(vals.startValue, vals.endValue, w, func); }

float mobilityProgressFunction(float x)
{
    if (x < 1.f / 3)
        return 500.f * sqrt(x) / 577.f;
    else if (x <= 2.f / 3)
        return 1.f / 2 + 3.f * (x - 1.f / 3) / 5;
    else
        return 9.f * x * x / 10 - 3.f * x / 5 + 7.f / 10;
}