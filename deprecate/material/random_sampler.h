#ifndef RANDOM_SAMPLER_H
#define RANDOM_SAMPLER_H

#include "sampler.h"
#include <random>
namespace cblt
{
    class RandomSampler : public Sampler
    {
    public:
        RandomSampler(unsigned int seed = 1U);
        void Next1D(float &x) override;
        void Next2D(float &x, float &y) override;
    private:
        std::mt19937 generator_;
        std::uniform_real_distribution<float> distribution_;
    };
}
#endif  // RANDOM_SAMPLER_H