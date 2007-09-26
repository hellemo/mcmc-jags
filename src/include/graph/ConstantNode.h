#ifndef CONSTANT_NODE_H_
#define CONSTANT_NODE_H_

#include <graph/Node.h>

/**
 * @short Top-level Node representing data
 *
 * Constant nodes are the top-level nodes in any directed acyclic graph.
 * They are always scalar, and have no parents.
 *
 * In the BUGS language. Constant nodes appear only on the right hand
 * side of a relation. They are considered to represent observed
 * random variables.
 */
class ConstantNode : public Node {
public:
    /**
     * Constructs a constant node and sets its value. The value is
     * fixed and is shared between all chains.
     */
    ConstantNode(double value, unsigned int nchain);
    /**
     * This function does nothing. It exists only so that objects of
     * class ConstantNode can be instantiated.
     */
    void deterministicSample(unsigned int);
    /**
     * This function does nothing. The value of the constant node is
     * not changed and the state of the RNG remains the same.
     */
    void randomSample(RNG*, unsigned int);
    /**
     * Constant nodes have no parents. This function always returns true.
     */
    bool checkParentValues(unsigned int) const;
    /**
     * If a constant node does not have a name in the supplied
     * name table, then it is named after its value.
     */
    std::string name(NodeNameTab const &name_table) const;
    /**
     * Constant nodes are observed random variables. This function
     * returns true.
     */
    bool isRandomVariable() const;
    /**
     * Constant nodes are trivial fixed linear functions. This function
     * always returns true.
     */
    bool isLinear(std::set<Node const*> const &parameters, 
		  Graph const &graph, bool fixed) const;
    /**
     * Constant nodes are trivial fixed scale functions. This function
     * always returns true.
     */
    bool isScale(std::set<Node const*> const &parameters, 
		 Graph const &graph, bool fixed) const;
};

#endif /* CONSTANT_NODE_H_ */




