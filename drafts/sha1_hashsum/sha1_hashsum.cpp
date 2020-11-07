#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <string.h>
#include <byteswap.h>

#define INVAL_FD    -1
#define ENOERR      0

#define BUFFER_SIZE (80 * sizeof(uint32_t)) /* 512 bits */
#define CHANK_SIZE  64 /* 512 bits */
#define ML_SIZE     8

#define is_bigendian() (!(union { uint16_t u16; unsigned char c; }){ .u16 = 1 }.c)

#define leftrotate(_n, _offset)    ((_n << _offset) | (_n >> (8*(sizeof(_n)) - _offset)))

static unsigned int h0 = 0x67452301;
static unsigned int h1 = 0xEFCDAB89;
static unsigned int h2 = 0x98BADCFE;
static unsigned int h3 = 0x10325476;
static unsigned int h4 = 0xC3D2E1F0;

static void handle_chank(uint32_t *words) {

    unsigned int a = h0;
    unsigned int b = h1;
    unsigned int c = h2;
    unsigned int d = h3;
    unsigned int e = h4;

    if(!is_bigendian()) {
        for (int ix = 0; ix < 16; ix++) {
            words[ix] = htobe32(words[ix]);    
        }
    }
   
    for(int ix = 16; ix < 80; ++ix) {
        words[ix] = words[ix - 3] ^
                    words[ix - 8] ^ 
                    words[ix - 14] ^
                    words[ix - 16];
        words[ix] = leftrotate(words[ix], 1);
    }

    for (int i = 0; i < 80; i++) {

        uint32_t k = 0;
        uint32_t f = 0;

        if ((0 <= i) && (i <= 19)) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        }
        else if ((20 <= i) && (i <= 39)) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        }
        else if ((40 <= i) && (i <= 59)) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        }
        else if ((60 <= i) && (i <= 79)) {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        uint32_t temp = leftrotate(a, 5) + f + e + k + words[i];
        e = d;
        d = c;
        c = leftrotate(b, 30);
        b = a;
        a = temp;
    }

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
}

int main(int argc, const char** argv) {

    int rc = ENOERR;

    std::ifstream file;
    std::istream *from = nullptr;

    if (argc == 1) {
        // no arguments, read standard input
        from = &std::cin;
    }
    else if (argc == 2) {

        std::string arg = argv[1];

        if ((arg == "-") || (arg == "--")) {
            // FILE is just -, read standard input
            from = &std::cin;
        }
        else {
            file.open(argv[1], std::ios_base::in);
            if (false == file.is_open()) {
                std::cout << "Failed to open file '" << arg << "'. "
                          << strerror(errno) << " (rc=" << errno << ")."
                          << std::endl;
                rc = EINVAL;
            }
            from = &file;
        }
    }
    else {
        std::string arg = argv[0];
        std::cout << "Usage: " << arg.substr(arg.find_last_of("/\\") + 1) << " [FILE]" << std::endl
                  << "With no FILE, or when FILE is just -, read standard input.\n"  << std::endl;
        rc = EINVAL;
    }

    if (rc == ENOERR) {

        char chunk[BUFFER_SIZE] = {0};

        size_t ml = 0; /* message length */
        size_t cl = 0; /* buffer length */

        /* Add extra chank if there are no room for the bit '1' and message length */
        while ((!from->eof()) || (cl > (CHANK_SIZE - ML_SIZE - 1))) {

            from->read(chunk, CHANK_SIZE);
            ml += cl = from->gcount();

            /* Handle last chank, append the bit '1' and message length */
            if (cl != CHANK_SIZE) {

                memset(&chunk[cl], 0, CHANK_SIZE - cl);

                /* Append the bit '1' to the message */
                if (((cl != 0) && (ml % CHANK_SIZE != 0)) || 
                    ((cl == 0) && (ml % CHANK_SIZE == 0))) {

                    chunk[cl] = 0x80;
                }

                /* Append the original message length, as a 64-bit big-endian integer. */
                if ((cl == 0) || (cl < (CHANK_SIZE - ML_SIZE))) {

                    uint bpos = CHANK_SIZE - 1;
                    ml *= 8;
                    while (ml != 0) {
                        chunk[bpos--] = ml & 0xFF;
                        ml = ml >> 8;
                    }
                }
            }

            handle_chank((uint32_t*)chunk);
        }
    }    

    std::cout.fill('0');
    std::cout << std::hex << std::setw(8) << h0;
    std::cout << std::hex << std::setw(8) << h1;
    std::cout << std::hex << std::setw(8) << h2;
    std::cout << std::hex << std::setw(8) << h3;
    std::cout << std::hex << std::setw(8) << h4;
    std::cout << std::endl;

    return rc;
}