#include <iostream>
#include <set>
#include <map>
#include <string>

namespace WordChain {

class WordChain {
public:
    WordChain();
    virtual ~WordChain();

    void generate(uint32_t size);
    void print_words(std::ostream& os);

    void search();

private:
    std::map<uint32_t, std::string> words;
};

} // namespace wordchain