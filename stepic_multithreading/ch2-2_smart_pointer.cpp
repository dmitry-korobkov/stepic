#include <iostream>

class StringPointer {
private:
    std::string *ptr;
    std::string *dummy;
public:
    StringPointer(std::string *Pointer) : ptr(Pointer),
                                          dummy(nullptr) {
        if (ptr == nullptr) {
            ptr = dummy = new std::string();
        }
    }
    ~StringPointer() {
        if (dummy != nullptr) {
            delete dummy;
        }
    }
    std::string *operator->() { return ptr; }
    operator std::string*() { return ptr; }
};

int main(int argc, char **argv) {

    std::string s1 = "Hello, world!";
    StringPointer sp1(&s1);
    StringPointer sp2(NULL);

    std::cout << sp1->length() << std::endl;
    std::cout << *sp1 << std::endl;
    std::cout << sp2->length() << std::endl;
    std::cout << *sp2 << std::endl;

    return 0;
}