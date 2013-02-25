#ifndef D_FUNCTION_H_
#define D_FUNCTION_H_

#include "DPQFunction.h"

namespace jags {
namespace bugs {

    class DFunction : public DPQFunction
    {
    public:
	DFunction(RScalarDist const *dist);
	bool checkParameterValue(std::vector<double const *> const &args) const;
	double evaluate(std::vector <double const *> const &args) const;
    };

}}

#endif /* D_FUNCTION_H_ */
