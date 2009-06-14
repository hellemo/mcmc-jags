#include <config.h>

#include "ConjugateLMFactory.h"
#include "ConjugateLM.h"

#include <graph/StochasticNode.h>
#include <distribution/Distribution.h>

namespace glm {

    bool ConjugateLMFactory::checkOutcome(StochasticNode const *snode) const
    {
	return snode->distribution()->name() == "dnorm";
    }
    
    bool ConjugateLMFactory::checkLink(InverseLinkFunc const *link) const
    {
	return false;
    }

    GLMMethod *ConjugateLMFactory::newMethod() const
    {
	return new ConjugateLM;
    }

}