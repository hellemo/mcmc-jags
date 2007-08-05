#include <config.h>
#include <sarray/util.h>
#include <sarray/nainf.h>
#include "DMNorm.h"
#include "DMT.h"

#include <lapack.h>
#include <matrix.h>

#include <cmath>
#include <vector>
#include <stdexcept>

#include <JRmath.h>

using std::vector;
using std::logic_error;

DMT::DMT()
  : Distribution("dmt", 3, false, false) 
{}

double DMT::logLikelihood(double const *x, unsigned int m,
			  vector<double const *> const &parameters,
			  vector<vector<unsigned int> > const &dims) const
{
    double const * mu = parameters[0];
    double const * T = parameters[1];
    double k = parameters[2][0];

    /* Calculate inner product ip = t(x - mu) %*% T %*% (x - mu) */
    double ip = 0;
    double * delta = new double[m];
    for (unsigned int i = 0; i < m; ++i) {
	delta[i] = x[i] - mu[i];
	double const *Ti = T + i*m;
	ip += (delta[i] * Ti[i] * delta[i]);
	for (unsigned int j = 0; j < i; ++j) {
	    ip += 2 * delta[i] * Ti[j] * delta[j];
	}
    }
    delete [] delta;

    double d = m; // Avoid problems with integer division
    return -((k + d)/2) * (1 + ip/k) + logdet(T, m)/2 +
	lgamma((k + d)/2) - lgamma(k/2) - (d/2) * log(k) - (d/2) * log(M_PI);
}

void DMT::randomSample(double *x, unsigned int length,
		       vector<double const *> const &parameters,
		       vector<vector<unsigned int> > const &dims,
		       RNG *rng) const
{

    double const * mu = parameters[0];
    double const * T = parameters[1];
    double k = *parameters[2];

    DMNorm::randomsample(x, mu, T, true, length, rng);
    double C = rchisq(k, rng);
    for (unsigned int i = 0; i < length; ++i) {
	x[i] /= C;
    }
}

bool DMT::checkParameterDim(vector<vector<unsigned int> > const &dims) const
{
  if (!isVector(dims[0]))
    return false;
  if (!isSquareMatrix(dims[1]))
    return false;
  if (dims[0][0] != dims[1][0])
    return false;
  if (!isScalar(dims[2]))
      return false;
  return true;
}

vector<unsigned int> DMT::dim(vector<vector<unsigned int> > const &dims) const
{
    return dims[0];
}

bool
DMT::checkParameterValue(vector<double const *> const &parameters,
			    vector<vector<unsigned int> > const &dims) const
{
    unsigned int n = dims[0][0];
    double const *T = parameters[1];
    double k = *parameters[2];

    if (k < 2)
	return false;

    // Check symmetry
    for (unsigned int i = 1; i < n; i++) {
	for (unsigned int j = 0; j < i - 1; j++) {
	    if (fabs(T[i + j*n] - T[j + i*n]) > DBL_EPSILON)
		return false;
	}
    }
    // Don't bother checking positive definiteness

    return true;
}


void DMT::support(double *lower, double *upper, unsigned int length,
		     vector<double const *> const &parameters,
		     vector<vector<unsigned int> > const &dims) const
{
    for (unsigned int i = 0; i < length; ++i) {
	lower[i] = JAGS_NEGINF;
	upper[i] = JAGS_POSINF;
    }
}

void DMT::typicalValue(double *x, unsigned int length,
			  vector<double const *> const &parameters,
			  vector<vector<unsigned int> > const &dims) const
{
    for (unsigned int i = 0; i < length; ++i) {
	x[i] = parameters[0][i];
    }
}

bool DMT::isSupportFixed(vector<bool> const &fixmask) const
{
    return true;
}