#include <cstdlib>
#include <iostream>
#include <ctime>

#include <word-chain.h>
#include <word-chain-graph.h>

namespace WordChain {

WordChain::WordChain() {}
WordChain::~WordChain() {}

void WordChain::generate(uint32_t size) {

    char first, last;

    std::srand(std::time(nullptr));
    last = (char)('a' + std::rand() % 26);

    for (uint32_t id = 0; id < size; ++id) {

        std::string word;

        std::swap(first, last);
        last = (char)('a' + std::rand() % 26);

        word.push_back(first);
        word.push_back(last);

        this->words.insert({id, word});
    }
}

void WordChain::print_words(std::ostream& os) {

    os << "Words:    ";
    for (auto it = words.begin(); it != words.end(); ++it) {
        os << it->second;
        if (std::next(it) != words.end()) {
            os << ",";
        }
    }
    os << ";" << std::endl;
}

void WordChain::search() {

    Graph *pGraph = new Graph(this->words);

    for (uint32_t ix = 1; ix < words.size(); ++ix) {
        pGraph = new Graph(this->words, pGraph);
        std::cout << ix << std::endl;
    }

    pGraph->print(std::cout);
    pGraph->cuttage();

    delete pGraph;
}

} // namespace wordchain