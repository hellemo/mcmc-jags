#include <config.h>
#include <rng/RNG.h>
#include <sampler/SingletonGraphView.h>
#include <graph/StochasticNode.h>
#include <graph/DeterministicNode.h>
#include <sampler/Linear.h>
#include <util/nainf.h>

#include "SumMethod.h"
#include <cmath>
#include <numeric>
#include <cfloat>
#include <stdexcept>

using std::vector;
using std::accumulate;
using std::min;
using std::max;
using std::ceil;
using std::logic_error;
using std::floor;
using std::fabs;

namespace jags {
    
    namespace bugs {

	template<class T>
	double sumValue(vector<T*> const &x, unsigned int ch)
	{
	    double y = 0;
	    for (typename vector<T*>::const_iterator p = x.begin();
		 p != x.end(); ++p)
	    {
		double const *v = (*p)->value(ch);
		unsigned int n = (*p)->length();
		y = accumulate(v, v+n, y);
	    }
	    return y;
	}
	    
	StochasticNode const *
	SumMethod::isCandidate(StochasticNode *snode, Graph const &graph)
	{
	    //We can only sample scalar nodes
	    if (snode->length() != 1) return 0;
	    
	    SingletonGraphView gv(snode, graph);

	    /* Check stochastic children
	       
	       There must be a single stochastic child, which is
	       observed and has the distribution "sum".  
	    */
	    vector<StochasticNode*> const &schildren = gv.stochasticChildren();
	    if (schildren.size() != 1) return 0;
	    StochasticNode const *schild = schildren.front();
	    if (schild->distribution()->name() != "sum") return 0;
	    if (!isObserved(schild)) return 0;
	    
	    /* Deterministic descendants must be an additive function
	     * of snode
	     */
	    if (!checkAdditive(&gv, false)) return 0;

	    return schild;
	}
	
	bool SumMethod::canSample(vector<StochasticNode*> const &snodes,
				  Graph const &graph)
	{
	    if (snodes.size() < 2) return false;

	    //Are individual nodes candidates?
	    Node const *sumchild = isCandidate(snodes[0], graph);
	    if (sumchild == 0) return false;
	    for (unsigned int i = 1; i < snodes.size(); ++i) {
		if (isCandidate(snodes[i], graph) != sumchild) return false;
	    }

	    //Together are the nodes additive with a fixed intercept?
	    if (!checkAdditive(snodes, graph, true)) return false;

	    //Check consistency of discreteness
	    bool discrete = sumchild->isDiscreteValued();
	    for (unsigned int i = 0; i < snodes.size(); ++i) {
		if (snodes[i]->isDiscreteValued() != discrete) return false;
		if (snodes[i]->length() != 1) return false;
	    }
	    return true;
	}

	SumMethod::SumMethod(GraphView const *gv, unsigned int chain)
	    : MutableSampleMethod(), _gv(gv), _chain(chain),
	      _sum(gv->stochasticChildren()[0]->value(chain)[0]),
	      _discrete(gv->stochasticChildren()[0]->isDiscreteValued()),
	      _x(gv->length()), _i(0), _j(0), _sumdiff(0), _iter(0),
	      _width(2), _max(10), _adapt(true)
	{
	    
	    gv->getValue(_x, chain);
	    
	    if (gv->logLikelihood(chain) != 0) {
		//If initial values are inconsistent with outcome we
		//try to adjust them

		StochasticNode const *schild = gv->stochasticChildren()[0];

		// Calculate intercept (usually zero)
		double y = sumValue<const Node>(schild->parents(), chain);
		double sumx = sumValue<StochasticNode>(gv->nodes(), chain);
		double alpha = y - sumx;

		// Calculate target sum for sampled values
		double sumx_new = schild->value(chain)[0] - alpha;

		// Rescale
		unsigned int N = _x.size();
		vector<double> xnew;
		if (_discrete) {
		    xnew = vector<double>(N, floor(sumx_new/N));
		    double delta = sumx_new -
			accumulate(xnew.begin(), xnew.end(), 0);
		    xnew[N-1] += delta;
		}
		else {
		    xnew = vector<double>(N, sumx_new/N);
		}

		gv->setValue(xnew, chain);
		if (_gv->logLikelihood(chain) != 0) {
		    throw logic_error("SumMethod failed to fix initial values");
		}
		if (jags_finite(gv->logPrior(_chain))) {
		    _x = xnew; //Preserve changes
		}
		else {
		    gv->setValue(_x, chain); //Revert changes
		}		
	    }

	    //Check validity of initial values
	    gv->checkFinite(chain);
	}
	
	SumMethod::~SumMethod()
	{
	}

	void SumMethod::updateStep(RNG *rng)
	{
	    // Test current value
	    double g0 = logDensity();
	    if (!jags_finite(g0)) {
		return;
	    }

	    // Generate auxiliary variable
	    double z = g0 - rng->exponential();

	    //Discrete width
	    double width = _discrete ? ceil(_width) : _width;
	    
	    // Generate random interval of width "_width" about current value
	    double left = _discrete ? floor(rng->uniform() * (width + 1)) :
		rng->uniform() * width;
	    double xold = value();
	    double L = xold - left;
	    double R = L + width;

	    double lower = JAGS_NEGINF, upper = JAGS_POSINF;
	    getLimits(&lower, &upper);

	    // Stepping out 

	    // Randomly set number of steps in left and right directions,
	    // subject to the limit in the maximal size of the interval
	    int j = static_cast<int>(rng->uniform() * _max);
	    int k = _max - 1 - j;

	    if (L < lower) {
		L = lower;
	    }
	    else {
		setValue(L);
		while (j-- > 0 && logDensity() > z) {
		    L -= width;
		    if (L < lower) {
			L = lower;
			break;
		    }
		    setValue(L);
		}
	    }

	    if (R > upper) {
		R = upper;
	    }
	    else {
		setValue(R);
		while (k-- > 0 && logDensity() > z) {
		    R += width;
		    if (R > upper) {
			R = upper;
			break;
		    }
		    setValue(R);
		}
	    }

	    // Keep sampling from the interval until acceptance (the loop is
	    // guaranteed to terminate).
	    double xnew;
	    for(;;) {
		if (_discrete) {
		    xnew =  L + floor(rng->uniform() * (R - L + 1));
		}
		else {
		    xnew =  L + rng->uniform() * (R - L);
		}
		setValue(xnew);
		double g = logDensity();
		if (g >= z - DBL_EPSILON) {
		    // Accept point
		    break;
		}
		else {
		    // shrink the interval
		    if (xnew < xold) {
			L = xnew;
		    }
		    else {
			R = xnew;
		    }
		}
	    }
	    
	    if (_adapt) {
		_sumdiff += fabs(xnew - xold);
	    }
	}

	void SumMethod::update(RNG *rng)
	{
	    unsigned int len = _gv->length();
	    for(_i = 0; _i < len; ++_i) {
		_j = static_cast<unsigned int>(rng->uniform() * (len-1));
		if (_j >= _i) _j++;
		updateStep(rng);
	    }

	    
	    if (_adapt) {
		if (++_iter % 50 == 0) {
		    _width = _sumdiff / (50 * len);
		    _sumdiff = 0;
		    if (_discrete) {
			_width = ceil(_width);
		    }
		}
	    }

	    //updateStep does not update the deterministic descendants
	    //but we need to leave these in a correct state in case they
	    //are being monitored
	    vector<DeterministicNode*> const &d = _gv->deterministicChildren();
	    for(vector<DeterministicNode*>::const_iterator p = d.begin();
		p != d.end(); ++p)
	    {
		(*p)->deterministicSample(_chain);
	    }

	    //Sanity check
	    if (_gv->logLikelihood(_chain) != 0) {
		throw logic_error("Failure to preserve sum in SumMethod");
	    }

	}

	void SumMethod::setValue(double x)
	{
	    double delta = x - _x[_i];
	    _x[_i] = x;      
	    _x[_j] -= delta; 

	    _gv->nodes()[_i]->setValue(&_x[_i], 1, _chain);
	    _gv->nodes()[_j]->setValue(&_x[_j], 1, _chain);
	}

	double SumMethod::value() const
	{
	    return _x[_i];
	}

	void SumMethod::getLimits(double *lower, double *upper) const
	{
	    vector<StochasticNode *> const &n = _gv->nodes();
	    double li, ui, lj, uj;
	    n[_i]->support(&li, &ui, 1U, _chain);
	    n[_j]->support(&lj, &uj, 1U, _chain);
	    double sum_ij = n[_i]->value(_chain)[0] + n[_j]->value(_chain)[0];
	    *lower = max(li, sum_ij - uj);
	    *upper = min(ui, sum_ij - lj);
	}

	bool SumMethod::isAdaptive() const
	{
	    return true;
	}
	
	void SumMethod::adaptOff()
	{
	    _adapt=false;
	}

	double SumMethod::logDensity() const
	{
	    /* 
	       Log density depends only on the prior of the two nodes
	       that are currently active
	    */
	    return _gv->nodes()[_i]->logDensity(_chain, PDF_PRIOR) +
		_gv->nodes()[_j]->logDensity(_chain, PDF_PRIOR);
	}

	bool SumMethod::checkAdaptation() const
	{
	    return true;
	}

    } // namespace bugs
} //namespace jags

