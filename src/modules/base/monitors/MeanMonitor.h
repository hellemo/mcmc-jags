#ifndef MEAN_MONITOR_H_
#define MEAN_MONITOR_H_

#include <model/Monitor.h>

#include <vector>

namespace jags {
namespace base {

    /**
     * @short Stores running mean of a given Node
     */
    class MeanMonitor : public Monitor {
	std::vector<std::vector<double> > _values; // sampled values
	unsigned int _n;
    public:
	MeanMonitor(Node const *node);
	void update();
	std::vector<double> const &value(unsigned int chain) const;
	std::vector<unsigned int> dim() const;
	bool poolChains() const;
	bool poolIterations() const;
    };

}}

#endif /* MEAN_MONITOR_H_ */
