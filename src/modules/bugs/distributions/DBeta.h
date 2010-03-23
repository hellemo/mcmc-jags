#ifndef DBETA_H_
#define DBETA_H_

#include <distribution/RScalarDist.h>

/**
 * @short Beta distribution
 * <pre>
 * P ~ dbeta(a, b)
 * f(p|a,b) = C(a,b) * p^(a -1) * (1 - p)^(b - 1) ; 0 < p < 1
 * </pre>
 */
class DBeta : public RScalarDist {
 public:
  DBeta();

  double d(double x, std::vector<double const *> const &parameters, 
	   bool give_log) const;
  double p(double q, std::vector<double const *> const &parameters, bool lower,
	   bool give_log) const;
  double q(double p, std::vector<double const *> const &parameters, bool lower,
	   bool log_p) const;
  double r(std::vector<double const *> const &parameters, RNG *rng) const;
  /**
   * Checks that a > 0, b > 0
   */
  bool checkParameterValue(std::vector<double const *> const &parameters) const;

};

#endif /* DBETA_H_ */
