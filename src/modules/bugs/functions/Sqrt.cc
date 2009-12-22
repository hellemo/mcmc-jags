#include <config.h>
#include "Sqrt.h"

#include <cmath>

using std::vector;
using std::sqrt;

namespace bugs {

    Sqrt::Sqrt ():ScalarFunc ("sqrt", 1)
    {
    }

    double Sqrt::evaluateScalar(vector<double const *> const &args) const
    {
	return sqrt(*args[0]);
    }

    bool Sqrt::checkScalarValue(vector<double const *> const &args) const
    {
	return *args[0] >= 0;
    }

    bool Sqrt::isPower(std::vector<bool> const &mask,
                       std::vector<bool> const &fix) const
    {
        return true;
    }


}