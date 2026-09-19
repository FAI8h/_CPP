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

    ~MemoryPool(){
        ::operator delete(buffer);
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

    void deallocate(T* ptr){
        char *p = reinterpret_cast<char *>(ptr);
        if (this->buffer > p || this->buffer + this->capacity <= p){
            throw std::range_error("bad pointer access");
        }

        *reinterpret_cast<void **>(ptr) = this->freeListHead;
        this->freeListHead = ptr;
    }
};

struct Widget {
    int id;
    Widget(int i) : id(i) {}
};

int main(){
    MemoryPool<Widget> pool(3); // 3 slots

    Widget* a = pool.allocate();
    a->id = 1;
    Widget* b = pool.allocate();
    b->id = 2;
    Widget* c = pool.allocate();
    c->id = 3;

    cout << "a=" << a->id << " b=" << b->id << " c=" << c->id << endl;
    cout << "a addr: " << (void*)a << ", b addr: " << (void*)b << ", c addr: " << (void*)c << endl;

    // pool should be exhausted now
    try {
        Widget* d = pool.allocate();
        cout << "ERROR: should have thrown, got " << (void*)d << endl;
    } catch (const std::exception& e) {
        cout << "correctly threw on exhaustion: " << e.what() << endl;
    }

    // free b, then allocate again - should get b's exact old address back (reuse!)
    void* bAddrBefore = (void*)b;
    pool.deallocate(b);
    Widget* reused = pool.allocate();
    cout << "reused slot addr: " << (void*)reused << " (should match b's old addr " << bAddrBefore << "): "
         << ((void*)reused == bAddrBefore ? "MATCH" : "MISMATCH") << endl;

    // test bad pointer rejection
    Widget stackWidget(99);
    try {
        pool.deallocate(&stackWidget); // not from this pool at all
        cout << "ERROR: should have thrown on foreign pointer" << endl;
    } catch (const std::exception& e) {
        cout << "correctly rejected foreign pointer: " << e.what() << endl;
    }

    return 0;
}