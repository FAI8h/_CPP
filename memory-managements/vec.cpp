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
    size_t size;
    size_t capacity;

    void grow(){
    size_t newCap = this->capacity * 2;
    auto newArr = allocator.allocate(newCap);

    int i = 0;
    try{
        for (; i < size; i++){
            allocator.construct(&newArr[i], std::move(data[i]));
        }
    }catch(const std::exception& e){

        for (int j = 0; j < i; j++){
            allocator.destroy(&newArr[j]);
        }
        allocator.deallocate(newArr);

        std::cerr << e.what() << '\n';
        throw;
    }

    /*
        * int i is the tracker if any thing goes wrong , i points to the exact point
        * when an error occurs we can undo the allocation of newArray(temporary array) , moving less that i times
        * and at last throw the error , so that exuction stops ,if not stopped the original data that havnt moved yet will get destroyed 
    */

    for (int i = 0; i < size; i++){
        allocator.destroy(&data[i]);
    }

    allocator.deallocate(data);
    this->capacity = newCap;
    data = newArr;
    }

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

    void push_back(T && val){
        if(size == capacity){
            grow();
        }
        allocator.construct(&data[size], std::move(val));
        size++;
    }
    void push_back(const T & val){
        if(size == capacity){
            grow();
        }
        allocator.construct(&data[size], val);
        size++;
    }

    template<typename... Args>
    void emplace_back(Args && ...args){
        if(size == capacity){
            grow();
        }

        allocator.construct(&data[size], std::forward<Args>(args)...);
        size++;
    }
    

    void pop_back(){
        if (size == 0){
            return;
        }

        allocator.destroy(&data[size - 1]);
        size--;
    }
};

int main(){
    MyVector<int> v;

    v.push_back(10);
    v.push_back(20);

    v.pop_back();
    cout << v[1] << endl;

    return 0;
}