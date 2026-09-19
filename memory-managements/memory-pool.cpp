#include <iostream>


using namespace std;

template<typename T>
class MemoryPool {
private:
    char *buffer;
    size_t capacity;
    void *freeListHead;

public:
    explicit MemoryPool(size_t numSlots){
        size_t slotSize = max(sizeof(T), sizeof(void *));

        this->capacity = slotSize * numSlots;
        this->buffer = static_cast<char *>(operator new(slotSize * numSlots));

        int i = 0;
        void **addr = nullptr;
        for (; i < numSlots - 1; i++){
            //addr
            addr = reinterpret_cast<void **>(buffer + i * slotSize);
            *(addr) = reinterpret_cast<void *>(buffer + (i + 1) * slotSize);
        }
        addr = reinterpret_cast<void **>(buffer + i * slotSize);
        *(addr) = nullptr;

        this->freeListHead = this->buffer;
    }

    T* allocate(){
        if(this->freeListHead == nullptr){
            throw std::out_of_range("pool exhausted");
        }

        void *slot = this->freeListHead;

        void * next = *reinterpret_cast<void **>(slot);

        this->freeListHead = next;

        return reinterpret_cast<T *>(slot);
    }
};

int main(){

    return 0;
}