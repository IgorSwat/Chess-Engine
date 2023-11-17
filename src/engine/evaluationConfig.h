#pragma once

#include <cinttypes>
#include <functional>
#include <cmath>


using Value = std::int16_t;

namespace Evaluation {

    struct Parameter
    {
        Value opening;
        Value endgame;
    };

    enum EvalParameters {
        PAWN_BASE_VALUE = 0,
        KNIGHT_BASE_VALUE,
        BISHOP_BASE_VALUE,
        ROOK_BASE_VALUE,
        QUEEN_BASE_VALUE,
        KNIGHT_PAWNS_BONUS,
        KNIGHT_DISTANT_PAWNS_PENALTY,
        KNIGHT_OUTPOST_I_DEG_BONUS,
        KNIGHT_OUTPOST_II_DEG_BONUS,
        KNIGHT_OUTPOST_III_DEG_BONUS,
        KNIGHT_MOBILITY_ZERO,
        KNIGHT_MOBILITY_FULL,
        BISHOP_PAIR_BONUS,
        BAD_BISHOP_PENALTY,
        BISHOP_FIANCHETTO_BONUS,
        BISHOP_COLOR_WEAKNESS_BONUS,
        BISHOP_OUTPOST_I_DEG_BONUS,
        BISHOP_OUTPOST_II_DEG_BONUS,
        BISHOP_OUTPOST_III_DEG_BONUS,
        BISHOP_MOBILITY_ZERO,
        BISHOP_MOBILITY_FULL,
        ROOK_ON_SEMIOPEN_FILE_BONUS,
        ROOK_ON_OPEN_FILE_BONUS,
        ROOK_ON_78_RANK_BONUS,
        ROOK_UNDEVELOPED_PENALTY,
        ROOK_MOBILITY_ZERO,
        ROOK_MOBILITY_FULL,
        QUEEN_MOBILITY_ZERO,
        QUEEN_MOBILITY_FULL,

        EVAL_PARAMETERS_RANGE = 33
    };

    enum OutpostType {
        I_DEG_OUTPOST = 0, II_DEG_OUTPOST, III_DEG_OUTPOST,

        OUTPOST_TYPE_RANGE = 3
    };


    extern Parameter EVALUATION_CONFIG[EVAL_PARAMETERS_RANGE];

    inline const Parameter& param(EvalParameters parameter)
    {
        return EVALUATION_CONFIG[parameter];
    }



    // m should be in range of [0, 1]
    template <typename Functional>
    inline Value interpolate(Value startValue, Value endValue, Functional func, float m)
    {
        return startValue + Value(func(m) * (endValue - startValue));
    }

    template <typename Functional>
    inline Value interpolate(const Parameter& parameter, Functional func, float m)
    {
        // High values of m (close to 1.0) mean a lot of pieces and mark position as opening
        return interpolate(parameter.endgame, parameter.opening, func, m);
    }

    class SigmoidalFunction
    {
    public:
        SigmoidalFunction(std::function<float(float)> function)
            : baseFunction(function) {}

        float operator()(float x) const
        {
            return x < 0.5f ? baseFunction(x) : 1.f - baseFunction(1.f - x);
        }

    private:
        std::function<float(float)> baseFunction;
    };

    constexpr auto linearFunction = [](float x){return x;};
    constexpr auto quadraticFunction = [](float x){return x * x;};
    constexpr auto squareRootFunction = [](float x){return std::sqrt(x);};
    const SigmoidalFunction sigmoidLowAbrupt = SigmoidalFunction([](float x){return 2.f * x * x;});
    const SigmoidalFunction sigmoidMidAbrupt = SigmoidalFunction([](float x){return 4.f * x * x * x;});
    const SigmoidalFunction sigmoidHighAbrupt = SigmoidalFunction([](float x){return 16.f * x * x * x * x * x;});
    const SigmoidalFunction reversedSigmoidLowAbrupt = SigmoidalFunction([](float x){return std::sqrt(x / 2.f);});
    const SigmoidalFunction reversedSigmoidMidAbrupt = SigmoidalFunction([](float x){return std::pow(x / 4.f, 1.f / 3.f);});
    const SigmoidalFunction reversedSigmoidHighAbrupt = SigmoidalFunction([](float x){return std::pow(x / 16.f, 1.f / 5.f);});
}