#include <config.h>
#include <graph/GraphMarks.h>
#include <graph/Graph.h>
#include <graph/Node.h>

#include <vector>
#include <set>
#include <stdexcept>
#include <utility>
#include <list>

using std::map;
using std::vector;
using std::set;
using std::logic_error;
using std::pair;
using std::list;

namespace jags {

GraphMarks::GraphMarks(Graph const &graph)
    : _graph(graph)
{
}

GraphMarks::~GraphMarks()
{}

Graph const &GraphMarks::graph() const
{
    return _graph;
}

void GraphMarks::mark(Node const *node, int m)
{
    if (!_graph.contains(node)) {
	throw logic_error("Attempt to set mark of node not in graph");
    }
    if (m == 0) {
	_marks.erase(node);
    }
    else {
	_marks[node] = m;
    }
}

int GraphMarks::mark(Node const *node) const
{
    if (!_graph.contains(node)) {
	throw logic_error("Attempt to get mark of node not in Graph");	    
    }
    
    map<Node const*, int>::const_iterator i = _marks.find(node);
    if (i == _marks.end()) {
	return 0;
    }
    else {
	return i->second;
    }
}

void GraphMarks::clear()
{
    _marks.clear();
}

void GraphMarks::markParents(Node const *node, int m)
{
    if (!_graph.contains(node)) {
	throw logic_error("Can't mark parents of node: not in Graph");
    }
    else {
	vector<Node const *> const &parents = node->parents();
	for (vector<Node const *>::const_iterator p = parents.begin(); 
	     p != parents.end(); ++p) 
	{
	    if (_graph.contains(*p)) {
		_marks[*p] = m;
	    }
	}
    }
}

//FIXME
//Used by MixtureSampler factory
void 
GraphMarks::markParents(Node const *node, bool (*test)(Node const *), int m)
{

    if (!_graph.contains(node)) {
	throw logic_error("Can't mark parents of node: not in Graph");
    }

    vector<Node const *> const &parents = node->parents();
    for (vector<Node const*>::const_iterator p = parents.begin(); 
	 p != parents.end(); ++p)
    {
	Node const *parent = *p;
	if (_graph.contains(parent)) {
	    if (test(parent)) {
		_marks[parent] = m;
	    }
	    else {
		markParents(parent, test, m);
	    }
	}
    }
}

void GraphMarks::markAncestors(vector<Node const *> const &nodes, int m)
{
    set<Node const*> visited; //visited nodes
    list<Node const*> marked; //marked nodes

    vector<vector<Node const*>::const_iterator> begin(1, nodes.begin());
    vector<vector<Node const*>::const_iterator> end(1, nodes.end());

    while (!begin.empty()) {

	for (vector<Node const*>::const_iterator &i = begin.back();
	     i != end.back(); ++i) 
	    {
	    
		Node const *inode = *i;
		if (visited.count(inode) == 0 && _graph.contains(inode)) {
		    visited.insert(inode);
		    marked.push_back(inode);
		    begin.push_back(inode->parents().begin());
		    end.push_back(inode->parents().end());
		    break;
		}
	    }

	if (begin.back() == end.back()) {
	    begin.pop_back();
	    end.pop_back();
	}
    }

    /* 
       When inserting a large number of nodes into the map, it is
       more efficient to insert them in order, as we get a hint
       about the correct placement from the last insertion
    */
    marked.sort();
    list<Node const*>::const_iterator p = marked.begin();
    map<Node const*,int>::iterator q = 
	_marks.insert(pair<Node const*const,int>(*p,m)).first;
    
    for(++p; p != marked.end(); ++p) {
	q = _marks.insert(q, pair<Node const*const,int>(*p,m));
    }

}

} //namespace jags
