#ifndef SOBOL2D_H
#define SOBOL2D_H
#include "sampler.h"
#include <vector>

namespace cblt
{

	/**
 	* @brief A 2D sobol sampler used to generate Quasi-Random numbers. Sampling ray directions using
 	* a Quasi-Random sequence allows for faster convergence in Monte-Carlo methods. Sampler is built
 	* following Joe and Kuo's recurrence relation and directional numbers (https://epubs.siam.org/doi/10.1137/070709359)
 	*/
	class SobolSampler : public Sampler
	{
		public:
		SobolSampler(unsigned long x_0 = 0, unsigned long y_0 = 0);
		void Next1D(float &x) override;
		void Next2D(float& x, float& y) override;
		
		private:
		float sobol_max_;  //! The maximum possible number this generator can create, used to max output from graycode to [0, 1]

		unsigned long sobol_x_n_ = 0;  //! The current sobol number in the x dimension
		unsigned long sobol_y_n_ = 0;  //! The current sobol number in the y dimension
		unsigned long sobol_n_ = 0;    //! The number of sobol numbers generated
		std::vector<unsigned long> m_i_s_;  //! The previous recurrence relation numbers
		std::vector<unsigned long> sobol_x_s_;  //! The previous x dimension sobol numbers
		std::vector<unsigned long> sobol_y_s_;  //! The previous y dimension sobol numbers
	};
}

#endif  // SOBOL2D_H

