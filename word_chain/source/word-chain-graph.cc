#include <cstdlib>
#include <iostream>
#include <ctime>

#include <word-chain-node.h>
#include <word-chain-graph.h>

namespace WordChain {

Graph::Graph(std::map<uint32_t, std::string> & words) : 
    depth (1),
    parent (nullptr) {

    for (auto const& word : words) {
        
        this->nodes.emplace_back(Node(word.first, depth, word.second.front(), word.second.back()));
    }
}

Graph::Graph(std::map<uint32_t, std::string> & words, Graph *parent)
    : Graph(words) {

    this->parent = parent;
    this->depth = parent->depth + 1;

    for (auto & node: this->nodes) {
        node.build(parent->nodes);
    }
    if (0 == this->depth % 10) {
        cuttage();
    }
}

Graph::~Graph() {
    if (nullptr != this->parent) {
        delete this->parent;
    }
}

void Graph::print(std::ostream& os) const {
    
    os << "Graph:" << std::endl;
    this->printNodes(os);
    //if (nullptr != parent) {
    //    parent->printParent(os);
    //}
}

void Graph::printParent(std::ostream& os) const {
    if (nullptr != parent) {
        parent->printNodes(os);
        parent->printParent(os);
    }
}

void Graph::printNodes(std::ostream& os) const {

    os << " -- " << this->depth << ": ";
    for (auto it = nodes.cbegin(); it != nodes.cend(); ++it) {
        if (it->getSize() == 0) 
            continue;
        os << it->getFirst() << it->getLast();
        os << ":" << it->getSize();
        if (std::next(it) != nodes.end()) {
            os << ", ";
        }
    }
    os << ";" << std::endl;
}

void Graph::cuttage() {

    if (nullptr != parent) {

        std::map<char, std::list<uint32_t>> refs;

        for (auto & node: this->nodes) {

            if (0 == node.getSize())
                continue;

            auto search = refs.find(node.getFirst());

            if (search != refs.end()) {
                search->second.push_back(node.getId());
            }
            else{
                std::list<uint32_t> list;
                list.push_back(node.getId());
                refs.insert({node.getFirst(), list});
            }
        }

        for (auto & node: parent->nodes) {
            if (0 == node.getSize())
                continue;
            
            node.cuttage(refs.at(node.getFirst()));
        }
    }
}


} // namespace wordchain