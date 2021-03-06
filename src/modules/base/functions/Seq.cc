#include <config.h>
#include "Seq.h"

using std::vector;
using std::string;

namespace jags {
    namespace base {

	Seq::Seq() : VectorFunction(":", 2)
	{
	}

	void Seq::evaluate(double *value, 
			   vector <double const *> const &par_values,
			   vector <unsigned int> const &par_lengths) const
	{
	    int lhs = static_cast<int>(*par_values[0]);
	    int rhs = static_cast<int>(*par_values[1]);

	    if (rhs < lhs) return;

	    int N = rhs - lhs + 1;
	    for (int i = 0; i < N; ++i) {
		value[i] = lhs + i;
	    }
	}

	unsigned int Seq::length(vector <unsigned int> const &lengths,
				 vector <double const *> const &values) const
	{
	    int lhs = static_cast<int>(*values[0]);
	    int rhs = static_cast<int>(*values[1]);

	    if (rhs < lhs) {
		return 0;
	    }
	    else {
		return rhs - lhs + 1;
	    }
	}

	bool Seq::checkParameterLength(vector<unsigned int> const &len) const
	{
	    return (len[0] == 1) && (len[1] == 1);
	}

        bool Seq::isDiscreteValued(vector<bool> const &mask) const
	{
	    return true;
	}

        bool Seq::checkParameterDiscrete(vector<bool> const &mask) const
	{
	    //Both parameters must be discrete
	    return mask[0] && mask[1];
	}
	
	bool Seq::checkParameterFixed(vector<bool> const &mask) const
	{
	    //Both parameters must be fixed so that the length of the
	    //output vector is fixed
	    return mask[0] && mask[1];
	}

	string Seq::deparse(vector<string> const &par) const
	{
	    return par[0] + ":" + par[1];
	}

    } /* namespace base */
} /* namespace jags */

