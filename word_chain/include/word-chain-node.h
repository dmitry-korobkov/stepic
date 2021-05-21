#include <iostream>
#include <set>
#include <map>
#include <list>
#include <string>

#pragma once

namespace WordChain {

class Node {
public:
    Node(uint32_t id, uint32_t depth, char first, char last);
    virtual ~Node();

    const char& getFirst() const;
    const char& getLast() const;
    const uint32_t& getId() const;

    const std::size_t getSize() const;

    bool checkValidPath(uint32_t id) const;

    void build(std::list<Node> &nodes);
    void cuttage(std::list<uint32_t> &refs);


private:
    char first;
    char last;

    uint32_t id;
    uint32_t depth;

    std::list<Node*> nodes;
};

} // namespace wordchain