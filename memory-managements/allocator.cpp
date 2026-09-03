#include <iostream>
#include <memory>

using namespace  std;

template<typename T>

class MyAllocator{
public:
    //* this allocates the int size storage in the heap
    T* allocate(size_t count){
        return static_cast<T *>(::operator new(count * sizeof(T)));
    }

    //* this stores the object in that specefic location
    void construct(T* ptr,const T& val){
        new (ptr) T(val);
    }

    //* destroys the allocation pointer and memory
    void destroy(T* ptr){
        ptr->~T();
    }

    //* deallocates the pointer now pointer dosen't own storage in heap or point to
    void deallocate(T* ptr){
        ::operator delete(ptr);
    }
};

int main () {
    MyAllocator<int> allocator;

    int *memory = allocator.allocate(5);
    allocator.construct(memory, 42);
    allocator.construct(memory+1, 43);

    cout << memory+1 << endl;

    return 0;
}