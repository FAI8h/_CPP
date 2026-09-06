#include <iostream>

using namespace  std;



template<typename T>
class MyAllocator{
    public:
    //* this allocates the int size storage in the heap
    T* allocate(size_t count){
        return static_cast<T *>(::operator new(count * sizeof(T)));
    }
    
    //* this stores the object in that specefic location
    template <typename... Args>
    void construct(T* ptr,Args &&... args){
        new (ptr) T(std::forward<Args>(args)...);
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

template<typename T>
class MyVector {
private:
    MyAllocator<T> allocator;
    T *data;
    int size;
    int capacity;
public:
    MyVector(int capacity = 2) 
        : size(0), capacity(capacity){
            data = allocator.allocate(capacity);
        };


    T& operator[](int index){
        return data[index];
    }

    ~MyVector(){
        for (int i = 0; i < size; i++){
            allocator.destroy(&data[i]);
        }

        allocator.deallocate(data);
    }

    

    void push_back(T val){
        if(size == capacity){
            capacity = capacity * 2;
            auto newArr = allocator.allocate(capacity);

            for (int i = 0; i < size; i++){
                allocator.construct(&newArr[i], data[i]);
            }

            for (int i = 0; i < size; i++){
                allocator.destroy(&data[i]);
            }

            allocator.deallocate(data);

            data = newArr;
        }
        allocator.construct(&data[size], val);
        size++;
    }
};

int main(){
    MyVector<int> v;

    v.push_back(10);
    v.push_back(20);

    cout << v[0] << endl;

    return 0;
}