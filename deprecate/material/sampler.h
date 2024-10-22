#ifndef SAMPLER_H
#define SAMPLER_H
namespace cblt
{
    class Sampler 
    {
    public:
        virtual void Next1D(float &x) = 0;
        virtual void Next2D(float &x, float &y) = 0;
    };
}
#endif  // SAMPLER_H