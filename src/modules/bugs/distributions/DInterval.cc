#include <config.h>
#include "DInterval.h"
#include <util/dim.h>
#include <util/nainf.h>

#include <cfloat>
#include <algorithm>
#include <stdexcept>

using std::min;
using std::max;
using std::logic_error;
using std::vector;

#define T(par) (*par[0])
#define CUTPOINTS(par) (par[1])
#define NCUT(dims) (dims[1][0])

static unsigned int value(vector<double const *> const &par, unsigned int ncut)
{
    double t = T(par);
    for (unsigned int i = 0; i < ncut; ++i) {
	if (t <= CUTPOINTS(par)[i])
	    return i;
    }
    return ncut;
}

DInterval::DInterval()
    : Distribution("dinterval", 2, false, true)
{
}

bool 
DInterval::checkParameterDim(vector<vector<unsigned int> > const &dims) const
{
    return isScalar(dims[0]) && dims[1].size() == 1;
}

bool DInterval::checkParameterValue(vector<double const *> const &par,
				    vector<vector<unsigned int> > const &dims) 
    const
{
    for (unsigned int i = 1; i < NCUT(dims); ++i) {
	if (CUTPOINTS(par)[i] <= CUTPOINTS(par)[i-1])
	    return false;
    }
    return true;
}

double 
DInterval::logLikelihood(double const *y, unsigned int length, 
			 vector<double const *> const &par,
			 vector<vector<unsigned int> > const &dims,
			 double const *lower, double const *upper) const
{
    if (*y < 0)
	return JAGS_NEGINF;
    
    unsigned int x = static_cast<unsigned int>(*y);
    if (x > NCUT(dims)) {
	return JAGS_NEGINF;
    }
    else {
	double t = T(par);
	if (x > 0 && t <= CUTPOINTS(par)[x-1])
	    return JAGS_NEGINF;
	else if (x < NCUT(dims) && t > CUTPOINTS(par)[x])
	    return JAGS_NEGINF;
	else
	    return 0;
    }
}

void DInterval::randomSample(double  *x, unsigned int length,
			     vector<double const *> const &par,
			     vector<vector<unsigned int> > const &dims,
			     double const *lower, double const *upper,
			     RNG *rng) const
{
    /* 
       The random sample from DInterval is not random at all,
       but deterministic.
    */
    *x = static_cast<double>(value(par, NCUT(dims)));
}

void DInterval::typicalValue(double *x, unsigned int length,
			     vector<double const *> const &par,
			     vector<vector<unsigned int> > const &dims,
			     double const *lower, double const *upper) const
{
    *x = static_cast<double>(value(par, NCUT(dims)));
}

unsigned int DInterval::df(vector<vector<unsigned int> > const &dims) const
{
    return 0;
}

void DInterval::support(double *lower, double *upper, unsigned int length,
			std::vector<double const *> const &par,
			vector<vector<unsigned int> > const &dims) const
{
    unsigned int y = value(par, NCUT(dims));    
    *lower = y;
    *upper = y;
}


bool DInterval::isSupportFixed(vector<bool> const &fixmask) const
{
    return fixmask[0] && fixmask[1];
}
