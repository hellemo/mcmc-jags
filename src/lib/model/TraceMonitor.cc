#include <config.h>
#include <model/TraceMonitor.h>
#include <graph/Node.h>

#include <stdexcept>
#include <algorithm>

using std::vector;

TraceMonitor::TraceMonitor(Node const *node, unsigned int start, 
			   unsigned int thin)
    : Monitor(node, start, thin), _values(node->nchain())
{
}

void TraceMonitor::doUpdate()
{
    Node const *snode = node();
    unsigned int nchain = snode->nchain();
    unsigned int length = snode->length();
    for (unsigned int ch = 0; ch < nchain; ++ch) {
	double const *node_value = snode->value(ch);
	for (unsigned int i = 0; i < length; ++i) {
	    _values[ch].push_back(node_value[i]);
	}
    }
}

vector<double> const &TraceMonitor::values(unsigned int chain) const
{
    return _values[chain];
}

vector<unsigned int> TraceMonitor::dim() const
{
    return node()->dim();
}

unsigned int TraceMonitor::nchain() const
{
    return node()->nchain();
}

void TraceMonitor::reserve(unsigned int niter)
{
    unsigned int N = (1 + niter / thin()) * node()->length();
    for (unsigned int ch = 0; ch < _values.size(); ++ch) {
        _values[ch].reserve(_values[ch].size() + N);
    }
}

