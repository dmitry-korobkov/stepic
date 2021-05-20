#include <cstdlib>
#include <iostream>
#include <ctime>

#include <word-chain.h>

namespace WordChain {

WordChain::WordChain() {
}

WordChain::~WordChain() {
}

void WordChain::generate(uint32_t size) {

    char first, last;

    std::srand(std::time(nullptr));
    last = (char)('a' + std::rand() % 26);

    while (size--) {

        std::string word;

        std::swap(first, last);
        last = (char)('a' + std::rand() % 26);

        word.push_back(first);
        word.push_back(last);

        words.insert(word);
    }
}

void WordChain::print_words(std::ostream& os) {

    for (std::string const& word : this->words) {
        os << word << std::endl;
    }
}

} // namespace wordchain