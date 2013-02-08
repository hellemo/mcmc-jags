#ifndef KL_TAB_H_
#define KL_TAB_H_

#include <string>
#include <map>

namespace jags {
namespace dic {

    class KL;
    
    class KLTab {
	std::map<std::string, KL*> _klmap;
    public:
	KLTab();
	~KLTab();
	KL const *find(std::string const &name) const;
    };

}}

#endif /* KL_TAB_H_ */
