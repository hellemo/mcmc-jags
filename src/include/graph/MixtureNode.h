#ifndef MIXTURE_NODE_H_
#define MIXTURE_NODE_H_

#include <graph/DeterministicNode.h>
#include <map>
#include <vector>

/**
 * @short Node for mixture models.
 *
 * A mixture node copies its value from one of several parents, based
 * on the current value of a vector of index nodes.
 *
 * In the BUGS language, mixture nodes are represented using nested
 * indexing. For example, in the deterministic relation
 *
 * <pre>y[i] <- x[ind[i]]</pre>
 *
 * y[i] is a mixture node if ind[i] is unobserved.  If the possible
 * values of ind[i] are 1...M, then the parents of y[i] are ind[i],
 * x[1], ... x[M].
 */
class MixtureNode : public DeterministicNode {
    std::map<std::vector<int>, Node const *> _map;
    unsigned int _Nindex;
public:
    /**
     * Constructs a MixtureNode. 
     * @param index Vector of discrete-valued scalar nodes
     * @param parameters Vector of pairs. Each pair associates a
     * possible value of the index nodes with a parent. The mixture node
     * copies its value from the parent that matches the current value
     * of the index.
     */
    MixtureNode(std::vector<Node const *> const &index,
		std::map<std::vector<int>, Node const *> const &mixmap);
    ~MixtureNode();
    /**
     * Calculates the value of the node based on the current value of the
     * index and the value of the corresponding parameter.
     */
    void deterministicSample(unsigned int chain);
    /**
     * Returns the number of index parameters.
     */
    unsigned int index_size() const;
    /**
     * A MixtureNode preserves linearity if none of the indices are
     * parameters. It is never a fixed linear function.
     */
    bool isLinear(std::set<Node const*> const &parameters, bool fixed) const;
    /**
     * A MixtureNode is a scale transformation if none of the indices
     * are parameters. It is never a fixed scale transformation.
     */
    bool isScale(std::set<Node const*> const &parameters, bool fixed) const;
    /**
     * Checks that the current value of the index variable corresponds
     * to a valid Node in the vector of parameters.
     */
    bool checkParentValues(unsigned int chain) const;
};

MixtureNode const *asMixture(Node const *node);
bool isMixture(Node const *node);

#endif /* MIXTURE_NODE_H_ */
