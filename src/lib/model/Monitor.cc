#include <config.h>
#include <model/Monitor.h>
#include <graph/Node.h>

#include <stdexcept>

using std::vector;
using std::invalid_argument;

Monitor::Monitor(Node const *node, unsigned int start, unsigned int thin)
    : _node(node), _start(0), _thin(thin), _niter(0)
{
   if (thin == 0) {
	throw invalid_argument("Illegal thinning interval");
    }
}

Monitor::~Monitor()
{}

Node const * Monitor::node() const
{
    return _node;
}

unsigned int Monitor::start() const
{
    return _start;
}

unsigned int Monitor::end() const
{
    return _start + _thin * _niter;
}

unsigned int Monitor::thin() const
{
    return _thin;
}

void Monitor::update(unsigned int iteration)
{
    if (iteration < _start || (iteration - _start) % _thin != 0) {
	return;
    }
    else {
	doUpdate();
	_niter++;
    }
}

unsigned int Monitor::niter() const
{
    return _niter;
}

