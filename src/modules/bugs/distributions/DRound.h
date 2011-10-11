#ifndef DROUND_H_
#define DROUND_H_

#include <distribution/ScalarDist.h>

namespace bugs {

/**
 * @short Rounded observations
 * <pre>
 * x ~ dround(t, d)
 * f(x|t,d) = 1 if x = t rounded to d decimal places
 *          = 0 otherwise
 * </pre>
 */
class DRound : public ScalarDist {
public:
    DRound();
  
    double logDensity(double const x, PDFType type,
		      std::vector<double const *> const &parameters,
		      double const *lower, double const *upper) const;
    double randomSample(std::vector<double const *> const &parameters,
			double const *lower, double const *upper, 
			RNG *rng) const;
    double typicalValue(std::vector<double const *> const &parameters,
			double const *lower, double const *upper) const;
    double l(std::vector<double const *> const &parameters) const;
    double u(std::vector<double const *> const &parameters) const;
    bool isSupportFixed(std::vector<bool> const &fixmask) const;
    unsigned int df(std::vector<unsigned int> const &lengths) const;
    bool checkParameterValue(std::vector<double const *> const &par) const;
    double KL(std::vector<double const *> const &par1,
	      std::vector<double const *> const &par2) const;
};

}

#endif /* DROUND_H_ */
