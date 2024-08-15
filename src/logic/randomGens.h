#pragma once

#include "bitboards.h"
#include <random>
#include <numeric>
#include <climits>


// --------------------------
// Random generator interface
// --------------------------

template <typename DataType>
class RandomGenerator
{
public:
	RandomGenerator(DataType s) : seed(s) {}
	virtual DataType random() = 0;

protected:
	DataType seed;
};


// ---------------------------------------
// Specialized generator - MagicsGenerator
// ---------------------------------------

class MagicsGenerator : public RandomGenerator<uint64_t>
{
public:
	MagicsGenerator(uint64_t seed) : RandomGenerator(seed) {}

	uint64_t random() override
	{
		seed ^= ((seed << 49) | (seed >> 15)) ^ ((seed << 24) | (seed >> 40));
		seed *= MULT_VAL;
		seed ^= seed >> 35;
		seed *= MULT_VAL;
		seed ^= seed >> 28;

		return seed;
	}

	uint64_t sparseRandom()
	{
		return random() & random() & random();
	}

private:
	static constexpr uint64_t MULT_VAL = 0x9FB21C651E98DF25;
};


// ----------------------------------------------
// Specialized generators - MersenneTwister based
// ----------------------------------------------

template <typename IntType>
class MersenneTwister : public RandomGenerator<IntType>
{
public:
	MersenneTwister(IntType seed)
		: RandomGenerator<IntType>(seed), generator(seed), 
		  distribution(std::numeric_limits<IntType>::min(), std::numeric_limits<IntType>::max()) {}

	IntType random() override
	{
		return distribution(generator);
	}

private:
	std::mt19937_64 generator;
	std::uniform_int_distribution<uint64_t> distribution;
};