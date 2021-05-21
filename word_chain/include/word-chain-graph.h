#include <iostream>
#include <set>
#include <map>
#include <list>
#include <string>

#include <word-chain-node.h>

namespace WordChain {

class Graph {
public:
    Graph(std::map<uint32_t, std::string> & words);
    Graph(std::map<uint32_t, std::string> & words, Graph *parent);
    virtual ~Graph();

    void print(std::ostream& os) const;

    void cuttage();

protected:
    void printNodes(std::ostream& os) const;
    void printParent(std::ostream& os) const;

private:
    uint32_t depth;

    std::list<Node> nodes;
    Graph *parent;
};

} // namespace wordchain