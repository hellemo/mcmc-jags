#ifndef ALBERT_CHIB_H_
#define ALBERT_CHIB_H_

#include "BinaryGLM.h"

namespace glm {

    /**
     * Conjugate sampler for normal linear models.
     */
    class AlbertChib : public BinaryGLM {
	bool _gibbs;
	bool _aux_init;
    public:
	AlbertChib(GraphView const *view, 
		   std::vector<GraphView const *> const &sub_views,
		   unsigned int chain, bool gibbs);
	std::string name() const;
	void update(RNG *rng);
    };
    
}

#endif /* ALBERT_CHIB_H_ */
