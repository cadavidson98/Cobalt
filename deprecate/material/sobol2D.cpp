#include "sobol2D.h"
#include <iostream>
#include <algorithm>
#ifdef _WIN32
    #include <intrin.h>
    #pragma intrinsic(_BitScanForward)
#endif

namespace cblt
{

    SobolSampler::SobolSampler(unsigned long x_0, unsigned long y_0) : 
        sobol_x_n_(x_0), sobol_y_n_(y_0), sobol_n_(0)
    {
    	sobol_max_ = std::pow(2.f, 32);
        // push back the initial values for the m_i recurrence relation
    	m_i_s_.push_back(0);
    	m_i_s_.push_back(1 << 31);
        sobol_x_s_.push_back(sobol_x_n_);
        sobol_y_s_.push_back(sobol_y_n_);
    }
    
    void SobolSampler::Next1D(float &x)
    {
        // TODO
    }

    /**
     * @brief Find the next 2D sobol numbers in the sequence
     * 
     * @param x output for the next x number in the sequence
     * @param y output for the next y number in the sequence
     */
    void SobolSampler::Next2D(float &x, float &y)
    {
        // find c, the index of the rightmost 0 in the binary representation
        // of index (1 indexed)
        unsigned long temp = sobol_n_;
        unsigned long c = 1;
        
        // Now use Bit Scan Forward to find location of first zero (which is really one)
        #if defined(__linux__)
        // Linux we can use inline assembly
        // now use bsf
        // Change all the ones to zeros (and zeros to ones)...
        temp ^= 0xffffffff;
        asm("\tbsf %1, %0\n"
            : "=r"(c)
            : "r" (temp));
        // sobol recurrence relation is 1 indexed, not 0
        // so we add 1.
        ++c;
        #elif defined(_WIN32)
        // Change all the ones to zeros (and zeros to ones)...
        temp ^= 0xffffffff;
        // Microsoft we use Bitscanforward
        _BitScanForward(&c, temp);
        // sobol recurrence relation is 1 indexed, not 0
        // so we add 1.
        ++c;
        #else
        while (temp & 0x01) {
            c++;
            temp >>= 1;
        }
        #endif
        
        // now calculate m
        // Note: in the paper, Kuo defines M_c as 2^c. However,
        // this calculation stores the sobol values in fractional
        // binary. Here we use 2^(32 - c), so it is easier to convert the
        // integer representation into a floating point value.
        unsigned long m_c = 1 << (32 - c);  // m_i == 1 in one dimension
        // finally XOR to get the final value
        sobol_x_n_ ^= m_c;
        // calculate y - finding m requires a little more work since we are one dimension higher
        while (c > m_i_s_.size() - 1) {
            // build all the m_i terms up to the current m_i, as they are needed for the recurrence relation
            unsigned long m_i_minus_1 = m_i_s_.back();
            unsigned long m_i = m_i_minus_1 ^ (m_i_minus_1 >> 1);
            m_i_s_.push_back(m_i);
        }
        sobol_y_n_ ^= m_i_s_[c];
        // increment the index
        ++sobol_n_;
        // bring it back to a useful float
        // max is 2 ^ 32
        if (sobol_n_ >= sobol_max_) {
            std::cerr << "OUT OF SOBOL" << std::endl;
            exit(1);
        }
        x = sobol_x_n_ / sobol_max_;
        y = sobol_y_n_ / sobol_max_;
    }
}