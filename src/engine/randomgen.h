#pragma once

#include <cinttypes>
#include <climits>
#include <numeric>
#include <random>


/*
    ---------- Random generators ----------

    A helper random number generation library
    - Defines generator type classes which produce random numbers in different ways
    - Contains both general (true) random number generators, as well as specyfic generators for given problem
*/

namespace Random {

    // ---------------------------------
	// Random number generator interface
	// ---------------------------------

    // Set of type-dependable interfaces which should be implemented by all below generators
    template <typename IntType>
    class Generator
    {
    public:
        Generator(IntType seed) : m_seed(seed) {}

        virtual IntType random() = 0;

    protected:
        IntType m_seed;
    };


    // ---------------------------------------
	// Universal generators - Mersenne-Twister
	// ---------------------------------------

    // Uses Mersenne-Twister generator to generate random numbers in range [IntType::min, IntType::max]
    template <typename IntType>
    class StandardGenerator : public Generator<IntType>
    {
    public:
        StandardGenerator(IntType seed) : Generator<IntType>(seed), m_generator(seed), 
                                          m_distribution(std::numeric_limits<IntType>::min(), std::numeric_limits<IntType>::max()) {}

        IntType random() override { return m_distribution(m_generator); }

    private:
        std::mt19937_64 m_generator;
	    std::uniform_int_distribution<uint64_t> m_distribution;
    };


    // -----------------------------------------
	// Specialized generators - magics generator
	// -----------------------------------------

    // Magics generator is a sparse generator, that generates sparse uin64_t numbers for the purpose of magics initialization
    // - To generate sparse numbers, we simply utilize multiple bitwise AND operations on random numbers
    class MagicsGenerator : public Generator<uint64_t>
    {
    public:
        MagicsGenerator(uint64_t seed) : Generator<uint64_t>(seed) {}

        // I don't know where did I get it from, but it is a borrowed external formula
        uint64_t random() override
        {
            static constexpr uint64_t MULT = 0x9FB21C651E98DF25;

            m_seed ^= ((m_seed << 49) | (m_seed >> 15)) ^ ((m_seed << 24) | (m_seed >> 40));
		    m_seed *= MULT;
		    m_seed ^= m_seed >> 35;
		    m_seed *= MULT;
		    m_seed ^= m_seed >> 28;

		    return m_seed;
        }

        uint64_t sparse_random()
        {
            return random() & random() & random();
        }
    };

}

