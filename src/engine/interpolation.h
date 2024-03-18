#pragma once

#include "evaluationConfig.h"
#include <cmath>


namespace Interpolation {

    using ::Evaluation::IValue;

    // -------------------------------
    // Generic interpolation functions
    // -------------------------------

    template <typename Valtype, typename Functional>
    inline Valtype interpolate(const Valtype& startValue, const Valtype& endValue, Functional func, float m)
    {
        return startValue + Value(func(m) * (endValue - startValue));
    }

    template <typename Functional>
    inline IValue interpolate(const IValue& startValue, const IValue& endValue, Functional func, float m)
    {
        Value opening = interpolate<Value, Functional>(startValue.opening, endValue.opening, func, m);
        Value endgame = interpolate<Value, Functional>(startValue.endgame(), endValue.endgame(), func, m);
        return IValue(opening, endgame);
    }


    // -------------------------------------
    // Context-based interpolation functions
    // -------------------------------------

    // Linear ( f(x) = 1 - x ) game-stage interpolation
    inline Value interpolate_gs(const IValue& parameter, std::uint16_t stage)
    {
        return parameter.opening + ((parameter.diff * (256 - stage)) >> 8);
    }


    // ---------------------------------------------
    // Predefined elementary interpolation functions
    // ---------------------------------------------

    constexpr auto linearFunction = [](float x) {return x; };
    constexpr auto quadraticFunction = [](float x) {return x * x; };
    constexpr auto squareRootFunction = [](float x) {return std::sqrt(x); };


    // ------------------------------------------
    // Predefined complex interpolation functions
    // ------------------------------------------

    class SigmoidalFunction
    {
    public:
        SigmoidalFunction(std::function<float(float)> function)
            : baseFunction(function) {}

        float operator()(float x) const{ return x < 0.5f ? baseFunction(x) : 1.f - baseFunction(1.f - x); }

    private:
        std::function<float(float)> baseFunction;
    };

    const SigmoidalFunction sigmoidLowAbrupt = SigmoidalFunction([](float x) {return 2.f * x * x; });
    const SigmoidalFunction sigmoidMidAbrupt = SigmoidalFunction([](float x) {return 4.f * x * x * x; });
    const SigmoidalFunction sigmoidHighAbrupt = SigmoidalFunction([](float x) {return 16.f * x * x * x * x * x; });
    const SigmoidalFunction reversedSigmoidLowAbrupt = SigmoidalFunction([](float x) {return std::sqrt(x / 2.f); });
    const SigmoidalFunction reversedSigmoidMidAbrupt = SigmoidalFunction([](float x) {return std::pow(x / 4.f, 1.f / 3.f); });
    const SigmoidalFunction reversedSigmoidHighAbrupt = SigmoidalFunction([](float x) {return std::pow(x / 16.f, 1.f / 5.f); });

}