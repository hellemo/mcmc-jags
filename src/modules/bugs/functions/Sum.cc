#include <config.h>
#include <util/logical.h>
#include "Sum.h"

using std::vector;

namespace bugs {

    Sum::Sum () : Function("sum", 1)
    {
    }

    void Sum::evaluate(double *x, vector <double const *> const &args,
		       vector<unsigned int> const &lengths,
		       vector<vector<unsigned int> > const &dims) const
    {
	double value = args[0][0];
	for (unsigned long i = 1; i < lengths[0]; ++i) {
	    value += args[0][i];
	}
	*x = value;
    }

    bool 
    Sum::checkParameterDim (vector<vector<unsigned int> > const &args) const
    {
	return true;
    }

    bool Sum::isDiscreteValued(vector<bool> const &mask) const
    {
	return allTrue(mask);
    }

    bool Sum::isLinear(vector<bool> const &mask, vector<bool> const &fix) const
    {
	return true;
    }
}
