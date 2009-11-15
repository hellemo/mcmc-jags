#ifndef ALBERT_CHIB_GIBBS_FACTORY_H_
#define ALBERT_CHIB_GIBBS_FACTORY_H_

#include "BinaryFactory.h"

namespace glm {

    /**
     * @shortFactory object for conjugate linear model sampler
     */
    class AlbertChibGibbsFactory : public BinaryFactory
    {
    public:
	AlbertChibGibbsFactory();
	BinaryGLM *newBinary(Updater const *updater, 
			     std::vector<Updater const *> const &sub_updaters,
			     unsigned int chain) const;
	bool trunc() const;
    };

}

#endif /* ALBERT_CHIB_GIBBS_FACTORY_H_ */
