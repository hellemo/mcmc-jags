#include <config.h>
#include <distribution/RScalarDist.h>
#include <rng/RNG.h>
#include <util/nainf.h>
#include <util/dim.h>

#include <cmath>
#include <algorithm>

using std::string;
using std::vector;
using std::log;
using std::min;
using std::max;

namespace jags {

double RScalarDist::calPlower(double lower, 
			      vector<double const*> const &parameters) const
{
    //P(X < lower)
    if (_discrete) lower -= 1;
    return p(lower, parameters, true, false);
}

double RScalarDist::calPupper(double upper,
			      vector<double const*> const &parameters) const
{
    //P(X <= upper)
    return p(upper, parameters, true, false);
}


RScalarDist::RScalarDist(string const &name, unsigned int npar, 
			 Support support, bool discrete)
  
    : ScalarDist(name, npar, support),  _support(support), _discrete(discrete),
      _npar(npar)
{
}

double 
RScalarDist::typicalValue(vector<double const *> const &parameters,
			  double const *lower, double const *upper) const
{
    double llimit = l(parameters), ulimit = u(parameters);
    double plower = 0, pupper = 1;
    
    if (lower) {
	llimit = max(llimit, *lower);
	plower = calPlower(llimit, parameters);
    }

    if (upper) {
	ulimit = min(ulimit, *upper);
	pupper = calPupper(ulimit, parameters);
    }
    
    double pmed = (plower + pupper)/2;
    double med = q(pmed, parameters, true, false);	

    //Calculate the log densities
    double dllimit = d(llimit, PDF_FULL, parameters, true);
    double dulimit = d(ulimit, PDF_FULL, parameters, true);
    double dmed = d(med, PDF_FULL, parameters, true);

    //Pick the median if it has the highest density, otherwise pick
    //a point near to (but not on) the boundary
    if (dmed >= dllimit && dmed >= dulimit) {
	return med;
    }
    else if (dulimit > dllimit) {
	return q(0.1 * plower + 0.9 * pupper, parameters, true, false);
    }
    else {
	return q(0.9 * plower + 0.1 * pupper, parameters, true, false);
    }
}

double 
RScalarDist::logDensity(double x, PDFType type,
			vector<double const *> const &parameters,
			double const *lower, double const *upper) const
{
    if (lower && x < *lower)
	return JAGS_NEGINF;
    if (upper && x > *upper)
	return JAGS_NEGINF;
    if (upper && lower && *upper < *lower)
	return JAGS_NEGINF;
    
    double loglik =  d(x, type, parameters, true);

    if (type != PDF_PRIOR && (lower || upper)) {
	//Normalize truncated distributions

	double ll = l(parameters);
	if (lower && *lower < ll) ll = *lower;
	if (_discrete) ll -= 1; //Adjustment for discrete valued distributions

	/* In theory, we just have to subtract log[P(lower <= X <=
           upper)] from the log likelihood. But we need to work around
           numerical problems. */

	bool have_lower = lower && p(ll, parameters, true, false) > 0;
	bool have_upper = upper && p(*upper, parameters, false, false) > 0;

	if (have_lower && have_upper) {
	    if (p(ll, parameters, false, false) < 0.5) {
		//Use upper tail
		loglik -= log(p(ll, parameters, false, false) -
			      p(*upper, parameters, false, false));
	    }
	    else {
		//Use lower tail
		loglik -= log(p(*upper, parameters, true, false) - 
			      p(ll, parameters, true, false));
	    }
	}
	else if (have_lower) {
	    loglik -= p(ll, parameters, false, true);
	}
	else if (have_upper) {
	    loglik -= p(*upper, parameters, true, true);
	}
    }

    return loglik;
}


double 
RScalarDist::randomSample(vector<double const *> const &parameters,
			  double const *lower, double const *upper,
			  RNG *rng) const
{
    if (!lower && !upper) {
	return r(parameters, rng);
    }

    double plower = lower ? calPlower(*lower, parameters) : 0;
    double pupper = upper ? calPupper(*upper, parameters) : 1;

    if (pupper - plower > 0.25) {
	//Rejection sampling if expected number of samples is 4 or less
	while (true) {
	    double y = r(parameters, rng);
	    if (lower && y < *lower) continue;
	    if (upper && y > *upper) continue;
	    return y;
	}
    }

    //Inversion
    //FIXME: We probably need to take care of tail behaviour here
    double u = plower + rng->uniform() * (pupper - plower);
    return q(u, parameters, true, false);
}

bool RScalarDist::canBound() const
{
    return true;
}

bool RScalarDist::isDiscreteValued(vector<bool> const &mask) const
{
    return _discrete;
}

bool RScalarDist::discrete() const
{
    return _discrete;
}

unsigned int RScalarDist::npar() const
{
    return _npar;
}

    double xlog0(double x, bool give_log) {
	if (x < 0) return JAGS_POSINF;
	else if (x > 0) return give_log ? JAGS_NEGINF : 0;
	else return give_log ? 0 : 1;
    }

}
