#include <config.h>

#include <string>

#include "GLMGenericFactory.h"

#include "AuxMixPoisson.h"
#include "AuxMixBinomial.h"
#include "NormalLinear.h"
#include "BinaryLogit.h"
#include "BinaryProbit.h"

#include "GLMBlock.h"
#include "GLMGibbs.h"

#include <graph/StochasticNode.h>
#include <graph/LinkNode.h>
#include <distribution/Distribution.h>
#include <sampler/GraphView.h>
#include <module/ModuleError.h>

using std::vector;

namespace jags {
namespace glm {

    GLMGenericFactory::GLMGenericFactory()
	: GLMFactory("glm::Generic")
    {}

    bool GLMGenericFactory::checkOutcome(StochasticNode const *snode) const
    {
	return NormalLinear::canRepresent(snode) ||
	    BinaryLogit::canRepresent(snode) ||
	    BinaryProbit::canRepresent(snode) ||
	    AuxMixPoisson::canRepresent(snode) ||
	    AuxMixBinomial::canRepresent(snode);

    }
    
    GLMMethod *
    GLMGenericFactory::newMethod(GraphView const *view,
			 vector<SingletonGraphView const *> const &sub_views,
			 unsigned int chain, bool gibbs) const
    {
	vector<Outcome*> outcomes;

	for (vector<StochasticNode *>::const_iterator 
		 p = view->stochasticChildren().begin();
	     p != view->stochasticChildren().end(); ++p)
	{
	    Outcome *outcome = 0;
	    if (NormalLinear::canRepresent(*p)) {
		outcome = new NormalLinear(*p, chain);
	    }
	    else if (BinaryLogit::canRepresent(*p)) {
		outcome = new BinaryLogit(*p, chain);
	    }
	    else if (BinaryProbit::canRepresent(*p)) {
		outcome = new BinaryProbit(*p, chain);
	    }
	    else if (AuxMixBinomial::canRepresent(*p)) {
		outcome = new AuxMixBinomial(*p, chain);
	    }
	    else if (AuxMixPoisson::canRepresent(*p)) {
		outcome = new AuxMixPoisson(*p, chain);
	    }
	    else {
		throwLogicError("Invalid outcome in GLMGenericFactory");
	    }
	    outcomes.push_back(outcome);
	}

	if (gibbs) {
	    return new GLMGibbs(view, sub_views, outcomes, chain);
	}
	else {
	    return new GLMBlock(view, sub_views, outcomes, chain);
	}
    }

}}
