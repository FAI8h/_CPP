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

    //* copy constructor
    MyVector(const MyVector & other){
        this->size = other.size;
        this->capacity = other.capacity;

        data = allocator.allocate(other.capacity);

        for (int i = 0; i < other.size; i++){
            allocator.construct(&data[i], other.data[i]);
        }

    }

    //* copy assignment
    MyVector &operator=(const MyVector & other){
        if(this == &other) return *this;

        for (int i = 0; i < size; i++){
            allocator.destroy(&data[i]);
        }
        allocator.deallocate(data);

        /*
         * clarification on why we need to destory old object and deallocate the memory 
         * if the copy assignment is being called for already assigned object then we have to clear the object copy the new one entierly
         * now on why deallocate , Why not use the freed object that we just did , cause what if the object we are copying from has larger capacity and size (!* THINK ABOUT IT)  
        */
        size = other.size;
        capacity = other.capacity;

        data = allocator.allocate(other.capacity);

        for (int i = 0; i < other.size; i++){
            allocator.construct(&data[i], other.data[i]);
        }

        return *this;
    }


    //* Move constructor
    MyVector(MyVector && other) noexcept {
        this->size = other.size;
        this->capacity = other.capacity;

        this->data = other.data;
        /*
         * this->data = other.data // this->data starts pointing to other.data and then other.data = nullptr so the ownership transfered to this->data
         * && means this object is and rvalue and about to die any way so to save computation we just stole the ownership of that -> (other) 
         *  noexcept is for fall back if data is croupted any where while moving then it fall backs to copying rather than throwing error
        */
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    //* Move assignment
    MyVector& operator=(MyVector && other) noexcept {
        if(this == &other) return *this;

        for (int i = 0; i < size; i++){
            allocator.destroy(&data[i]);
        }
        allocator.deallocate(data);

        size = other.size;
        capacity = other.capacity;
        data = other.data;

        other.data = nullptr;
        other.capacity = 0;
        other.size = 0;

        return *this;
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