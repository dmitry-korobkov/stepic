#include <iostream>
#include <set>
#include <string>

namespace WordChain {

class WordChain {
public:
    WordChain();
    virtual ~WordChain();

    void generate(uint32_t size);
    void print_words(std::ostream& os);
private:
    std::multiset<std::string> words;
};

} // namespace wordchain