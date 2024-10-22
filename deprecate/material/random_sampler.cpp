#include "random_sampler.h"
#include <random>
#include <ctime>

namespace cblt
{
    RandomSampler::RandomSampler(unsigned int seed)
    {
        generator_.seed(seed);
        distribution_ = std::uniform_real_distribution<float>(0.0f, 1.0f);
    }

    void RandomSampler::Next1D(float &x)
    {
        x = distribution_(generator_);
    }

    void RandomSampler::Next2D(float &x, float &y)
    {
        x = distribution_(generator_);
        y = distribution_(generator_);
    }
}