#include <config.h>
#include "GreaterThan.h"

#include <cmath>
#include <cfloat>

using std::vector;

namespace basefunctions {

GreaterThan::GreaterThan () : Infix (">")
{
}

double GreaterThan::eval (vector<double const *> const &args) const
{
  return *args[0] > *args[1];
}

bool GreaterThan::isDiscreteValued(std::vector<bool> const &mask) const
{
  return true;
}

}
