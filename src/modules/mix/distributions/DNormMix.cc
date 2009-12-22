#include <config.h>
#include "DNormMix.h"
#include <util/dim.h>
#include <util/nainf.h>

#include <cmath>

#include <JRmath.h>

using std::vector;

#define MU(par) (par[0])
#define TAU(par) (par[1])
#define PROB(par) (par[2])

namespace mix {

    DNormMix::DNormMix()
	: Distribution("dnormmix", 3, false, false)
    {}

    bool 
    DNormMix::checkParameterDim(vector<vector<unsigned int> > const &dims) const
    {
	if (dims[0][0] == 1)
	    return false; // Must be a mixture
   
	// Parameter dimensions must match (but they need not be vectors)
	return (dims[1] == dims[0]) && (dims[2] == dims[0]);
    }

    bool 
    DNormMix::checkParameterValue(vector<double const *> const &par,
				  vector<vector<unsigned int> > const &dims) 
	const
    {
	unsigned int Ncat = product(dims[0]);
	for (unsigned int i = 0; i < Ncat; ++i) {
	    if (TAU(par)[i] <= 0)
		return false;
	    if (PROB(par)[i] <= 0) 
		return false;
	}
	return true;
    }

    double DNormMix::logLikelihood(double const *x, unsigned int length,
				   vector<double const *> const &par,
				   vector<vector<unsigned int> > const &dims,
				   double const *lower, double const *upper) 
	const
    {
	unsigned int Ncat = product(dims[0]);
	double density = 0.0;
	double psum = 0.0;
	for (unsigned int i = 0; i < Ncat; ++i) {
	    density += PROB(par)[i] * dnorm(*x, MU(par)[i], 
					    1/sqrt(TAU(par)[i]), 0);
	    psum += PROB(par)[i];
	}
	return log(density) - log(psum);
    }

    void 
    DNormMix::randomSample(double *x, unsigned int length,
			   vector<double const *> const &par, 
			   vector<vector<unsigned int> > const &dims, 
			   double const *lower, double const *upper, RNG *rng) 
	const
    {
	unsigned long Ncat = product(dims[0]);

	// Rescale probability parameter
	double sump = 0;
	for (unsigned int i = 0; i < Ncat; ++i) {
	  sump += PROB(par)[i];
	}
	double p_rand = runif(0, 1, rng) * sump;
	sump = 0;

	// Select mixture component (r)
	unsigned int r = Ncat - 1;
	for (unsigned int i = 0; i < Ncat - 1; ++i) {
	    sump += PROB(par)[i];
	    if (sump > p_rand) {
		r = i;
		break;
	    }
	}

	// Now sample from conditional distribution of component r
	double ans = rnorm(MU(par)[r], 1/sqrt(TAU(par)[r]), rng);
	*x = ans;
    }

    void DNormMix::typicalValue(double *x, unsigned int length,
				vector<double const *> const &par,
				vector<std::vector<unsigned int> > const &dims,
				double const *lower, double const *upper) const
    {
	double const *mu = MU(par);
	unsigned int Ncat = product(dims[0]);

	unsigned int j = 0;
	for (unsigned int i = 1; i < Ncat; ++i) {
	    if (PROB(par)[i] > PROB(par)[j]) {
		j = i;
	    }
	} 
    
	*x = mu[j];
    }

    bool DNormMix::isSupportFixed(std::vector<bool> const &fixmask) const
    {
	return true;
    }

    void 
    DNormMix::support(double *lower, double *upper, unsigned int length,
		      vector<double const *> const &parameters,
		      vector<vector<unsigned int> > const &dims) const
    {
	*lower = JAGS_NEGINF;
	*upper = JAGS_POSINF;
    }

}