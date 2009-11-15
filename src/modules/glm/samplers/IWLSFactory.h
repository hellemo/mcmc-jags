#ifndef IWLS_FACTORY_H_
#define IWLS_FACTORY_H_

#include "GLMFactory.h"

namespace glm {

    /**
     * @shortFactory object for conjugate linear model sampler
     */
    class IWLSFactory : public GLMFactory
    {
    public:
	IWLSFactory();
	bool checkOutcome(StochasticNode const *snode,
			  LinkNode const *lnode) const;
	GLMMethod *newMethod(Updater const *updater, 
			     std::vector<Updater const *> const &sub_updaters,
			     unsigned int chain) const;
	bool canSample(StochasticNode const *snode) const;
    };

}

#endif /* IWLS_FACTORY_H_ */
