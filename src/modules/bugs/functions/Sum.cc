#include <config.h>
#include "Sum.h"
#include <util/logical.h>

using std::vector;

namespace jags {
namespace bugs {

    Sum::Sum () : ScalarVectorFunction("sum", 0)
    {
    }

    double Sum::evaluate(vector <double const *> const &args,
			 vector<unsigned int> const &lengths) const
    {
	double value = 0;
	for (unsigned int j = 0; j < args.size(); ++j) {
	    for (unsigned int i = 0; i < lengths[j]; ++i) {
		value += args[j][i];
	    }
	}
	return value;
    }

    bool Sum::checkParameterLength (vector<unsigned int> const &args) const
    {
	return true;
    }

    bool Sum::isDiscreteValued(vector<bool> const &mask) const
    {
	return allTrue(mask);
    }

    bool Sum::isScale(vector<bool> const &mask, vector<bool> const &fix) const
    {
	return true;
    }

}}
