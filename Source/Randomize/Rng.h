#pragma once

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace mididestruct
{

// Thin seeded-RNG wrapper so the randomizer's output is a pure, reproducible
// function of (input, params, seed) - never touches global RNG state.
class Rng
{
public:
    explicit Rng (uint64_t seed) : engine (seed) {}

    // Uniform integer in [0, exclusiveMax).
    int nextInt (int exclusiveMax)
    {
        std::uniform_int_distribution<int> dist (0, exclusiveMax - 1);
        return dist (engine);
    }

    // Uniform double in [0, 1).
    double nextDouble()
    {
        std::uniform_real_distribution<double> dist (0.0, 1.0);
        return dist (engine);
    }

    // Fisher-Yates partial shuffle: picks `count` distinct indices from
    // [0, populationSize) and returns them in ascending order.
    std::vector<int> pickDistinctSorted (int populationSize, int count)
    {
        std::vector<int> population (static_cast<size_t> (populationSize));

        for (int i = 0; i < populationSize; ++i)
            population[static_cast<size_t> (i)] = i;

        std::shuffle (population.begin(), population.end(), engine);
        population.resize (static_cast<size_t> (count));
        std::sort (population.begin(), population.end());
        return population;
    }

private:
    std::mt19937_64 engine;
};

inline uint64_t makeRandomSeed()
{
    std::random_device rd;
    return (static_cast<uint64_t> (rd()) << 32) ^ static_cast<uint64_t> (rd());
}

} // namespace mididestruct
