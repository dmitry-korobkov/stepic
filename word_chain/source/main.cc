#include <iostream>

#include <word-chain.h>

int main(int argc, char * argv[])
{
    WordChain::WordChain wc;

    wc.generate(1000);
    wc.print_words(std::cout);

    wc.search();

    return 0;
}