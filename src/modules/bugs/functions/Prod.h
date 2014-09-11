#ifndef FUNC_PROD_H_
#define FUNC_PROD_H_

#include <function/ScalarVectorFunction.h>

namespace jags {
namespace bugs {

    /**
     * @short Product of an array
     * Sum calculates the product of the elements of an array
     * @see Sum
     * <pre>
     * y <- prod(x[])
     * </pre>
     */
    class Prod : public ScalarVectorFunction
    {
    public:
	Prod ();
	double evaluate(std::vector <double const *> const &args,
			std::vector<unsigned int> const &lengths) const;
	bool checkParameterLength(std::vector<unsigned int> const &len) const;
	bool isDiscreteValued(std::vector<bool> const &mask) const;
    };

}}

#endif /* FUNC_PROD_H_ */
