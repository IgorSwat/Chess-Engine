#pragma once

#include "bitboards.h"
#include <random>
#include <numeric>
#include <climits>

template <typename DataType>
class RandomGenerator
{
public:
	RandomGenerator(DataType s) : seed(s) {}
	virtual DataType random() = 0;

protected:
	DataType seed;
};



class MagicsGenerator : public RandomGenerator<uint64_t>
{
public:
	MagicsGenerator(uint64_t s) : RandomGenerator(s) {}

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



class MersenneTwister64 : public RandomGenerator<uint64_t>
{
public:
	MersenneTwister64(int s)
		: RandomGenerator(s), generator(seed), distribution(0, std::numeric_limits<uint64_t>::max()) {}

	uint64_t random() override
	{
		return distribution(generator);
	}

private:
	std::mt19937_64 generator;
	std::uniform_int_distribution<uint64_t> distribution;
};