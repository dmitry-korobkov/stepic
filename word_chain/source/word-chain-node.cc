#include <cstdlib>
#include <iostream>
#include <ctime>

#include <word-chain-node.h>

namespace WordChain {

Node::Node(uint32_t id, uint32_t depth, char first, char last) :
    id (id),
    depth (depth),
    first (first),
    last (last) {
}

Node::~Node() {}

const char& Node::getFirst() const { return this->first; }
const char& Node::getLast() const { return this->last; }
const uint32_t& Node::getId() const { return this->id; }

const std::size_t Node::getSize() const { 
    return this->nodes.size();
}

bool Node::checkValidPath(uint32_t id) const {

    if (this->id == id) {
        return false;
    }

    if (0 == nodes.size()) {
        return true;
    }

    for (auto node: this->nodes) {
        if (true == node->checkValidPath(id)) {
            return true;
        }
    }

    return false;
}

void Node::build(std::list<Node> &nodes) {

    for (auto & node: nodes) {

        if (node.depth == 0)
            continue;

        if (this->first != node.last)
            continue;

        if (true == node.checkValidPath(this->id)) {
            this->nodes.push_back(&node);
        }
    }

    if (0 == this->nodes.size()) {
        this->depth = 0;
    }
}

void Node::cuttage(std::list<uint32_t> &refs) {

    auto node_it = nodes.begin();
    while (node_it != nodes.end()) {
    
        auto remove_it = node_it++;

        auto it = refs.cbegin();
        while (it != refs.cend()) {
            if (true == this->checkValidPath(*it))
                break;
            it++;
        }

        if (it == refs.cend()) {
            //std::cout << "remove" << std::endl;
            nodes.erase(remove_it);
        }
    }
}

} // namespace wordchain