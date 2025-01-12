#pragma once

#include "evaluationConfig.h"
#include <cmath>
#include <numbers>


namespace Interpolation {

    // -----------------------------
    // Single interpolaion functions
    // -----------------------------

    // Linear ( f(x) = 1 - x ) game-stage interpolation
    inline Value interpolate_gs(const Evaluation::IValue& parameter, std::uint16_t stage)
    {
        return parameter.opening + ((parameter.diff * (256 - stage)) >> 8);
    }


    // -----------------
    // Function families
    // -----------------

    class NormalizedSigmoid
    {
    public:
        NormalizedSigmoid(float A, float k, float x0) : A(A), k(k), x0(x0) {}

        float operator()(float x) const { return A / (1 + std::pow(std::numbers::e, -k * (x - x0))) -
                                                 A / (1 + std::pow(std::numbers::e, k * x0)); }

    private:
        const float A;
        const float k;
        const float x0;
    };

}