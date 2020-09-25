#include <iostream>
#include <cstring>

class SmallAllocator {
private:
    static const size_t buffer_size = 1048576;
    uint8_t buffer[buffer_size];

public:
    uint8_t *heap_end;
    uint8_t *heap_free;

    typedef size_t _header;

    inline size_t& _block_size(void* ptr) {
        return *(size_t*)((uint8_t*)ptr - sizeof(size_t));
    }

public:

    SmallAllocator() : heap_end(buffer), heap_free(nullptr) {}

    void *Alloc(unsigned int size) {
        
        void* ptr = nullptr;

        uint8_t* it = heap_free;
        uint8_t* prev = nullptr;

        // allocated block can't be less than sizeof(void*)
        size = (size > sizeof(void*)) ? size : sizeof(void*);
        size += (sizeof(void*) - size % sizeof(void*)) % sizeof(void*);

        /* check released blocks */
        while ((it != nullptr) && (_block_size(it) < size)) {
            prev = it;
            it = *(uint8_t**)it;
        }

        if (it == nullptr) {

            if ((sizeof(_header) + size) <= ((buffer + buffer_size) - (uint8_t*)heap_end)) {

                ptr = heap_end + sizeof(_header);
                heap_end += sizeof(_header) + size;

                _block_size(ptr) = size;
            }
        }
        else
        {
            if (_block_size(it) >= sizeof(void*) + sizeof(_header) + size) {

                // block is big enough to be splitted up 
                _block_size(it) = _block_size(it) - sizeof(_header) - size;

                ptr = it + _block_size(it) + sizeof(_header);
                _block_size(ptr) = size;
            }
            else {

                // block size is equal or little greater, update block list
                if (it == heap_free) {
                    /* pop from the top */
                    heap_free = *(uint8_t**)it;
                }
                else {
                    /* middle element */
                    prev = *(uint8_t**)it;
                }

                // do not change block size
                ptr = it;
            }   
        }

        // std::cout << "alloc   : " << (size_t*)ptr << " " << _block_size(ptr) << std::endl;

        return ptr;
    };
    
    void Free(void *ptr) {

        uint8_t* it = nullptr;
        uint8_t* prev = nullptr;

        // std::cout << "free    : " << ptr << " " << _block_size(ptr) << std::endl;

        // keep blocks sorted, find position to insert new block
        if (ptr > heap_free) {
            /* push block on the top */
            *(uint8_t**)ptr = heap_free;
            heap_free = (uint8_t*)ptr;
        }
        else {
            /* find block to push after */
            for (it = heap_free; *(uint8_t**)it > ptr; prev = it, it = *(uint8_t**)it) {}
            *(uint8_t**)ptr = *(uint8_t**)it;
            *(uint8_t**)it = (uint8_t*)ptr;
        }

        // join left-adjacent block
        if ((*(uint8_t**)ptr != nullptr) &&
            (*(uint8_t**)ptr + _block_size(*(uint8_t**)ptr) + sizeof(_header) == ptr)) {

            _block_size(*(uint8_t**)ptr) += _block_size(ptr) + sizeof(_header);

            if (heap_free == ptr) {
                heap_free = *(uint8_t**)ptr;
            }

            if (it != nullptr) {
                it = *(uint8_t**)ptr;
            }
        }

        // join right-adjacent block
        if ((it != nullptr) &&
            (*(uint8_t**)it + _block_size(*(uint8_t**)it) + sizeof(_header) == it)) {

            _block_size(*(uint8_t**)it) += _block_size(it) + sizeof(_header);

            if (heap_free == it) {
                heap_free = *(uint8_t**)it;
            }

            if (prev != nullptr) {
                prev = *(uint8_t**)it;
            }
        }

        /* release last block to the heap */
        if (heap_end == ((uint8_t*)heap_free + _block_size(heap_free))) {
            heap_end = (uint8_t*)heap_free - sizeof(_header);
            heap_free = *(uint8_t**)heap_free;
        }
    };

    void *ReAlloc(void *ptr, unsigned int size) {
        
        uint8_t *res = (uint8_t*)this->Alloc(size);
        
        if (res != nullptr) {
            memcpy(res, ptr, (size < _block_size(ptr)) ? size : _block_size(ptr));
            this->Free(ptr);
        }

        return res;
    };
};

void print_free(SmallAllocator &allocator) {

    std::cout << "heap_end: " << (size_t*)allocator.heap_end << std::endl;
    for (uint8_t *ptr = allocator.heap_free;
         ptr != nullptr;
         ptr = *(uint8_t**)ptr) {

        std::cout << "        - "
                  << (size_t*)(ptr) 
                  << " "
                  << allocator._block_size(ptr) 
                  << std::endl;
    }
}

int main(int argc, char **argv) {

/*
    SmallAllocator allocator;

    uint8_t *v1 = (uint8_t*)allocator.Alloc(32 * sizeof(uint8_t));
    uint8_t *v2 = (uint8_t*)allocator.Alloc(32 * sizeof(uint8_t));
    print_free(allocator);
    
    allocator.Free(v1);
    print_free(allocator);
    
    uint8_t *v3 = (uint8_t*)allocator.Alloc(16 * sizeof(uint8_t));
    print_free(allocator);

    allocator.Free(v3);
    print_free(allocator);

    allocator.Free(v2);
    print_free(allocator);

    SmallAllocator A1;
    int * A1_P1 = (int *) A1.Alloc(sizeof(int));
    A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
    A1.Free(A1_P1);
    print_free(A1);
*/
    SmallAllocator A2;
    std::cout << "A2" << std::endl;
    print_free(A2);
    int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
    print_free(A2);
    for(unsigned int i = 0; i < 10; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;
    int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
    print_free(A2);
    for(unsigned int i = 0; i < 10; i++) A2_P2[i] = -1;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;
    A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
    print_free(A2);
    for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;

    A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
    print_free(A2);
    for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << " " << i << std::endl;
    A2.Free(A2_P1);
    A2.Free(A2_P2);
    print_free(A2);

    return 0;
}