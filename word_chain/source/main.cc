#include <iostream>
#include <word-chain.h>

int main(int argc, char * argv[])
{
    WordChain::WordChain wc;

    wc.generate(10);
    wc.print_words(std::cout);

    return 0;
}