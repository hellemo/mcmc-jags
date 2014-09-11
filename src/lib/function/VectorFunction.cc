#include <config.h>
#include <function/VectorFunction.h>

using std::vector;
using std::string;

namespace jags {

VectorFunction::VectorFunction (string const &name, unsigned int npar)
    : Function(name, npar)
{
}

bool 
VectorFunction::checkParameterValue(vector<double const *> const &args,
				    vector<unsigned int> const &lengths) const
{
    return true;
}

bool 
VectorFunction::checkParameterLength(vector<unsigned int> const &arglen) const
{
    return true;
}

} //namespace jags
