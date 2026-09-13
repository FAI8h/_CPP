#include <iostream>

using namespace std;

template<typename T>
class UniquePtr{
private:
    T *ptr;

public:
    explicit UniquePtr(T* p = nullptr) : ptr(p) {}

    ~UniquePtr(){
        delete ptr;
    }

    //* deleting copy constructor
    UniquePtr(const UniquePtr &other) = delete;
    
    //* deleting copy assignment
    UniquePtr &operator=(const UniquePtr &other) = delete;
    
    //* move constructor
    UniquePtr(UniquePtr&& other) noexcept{
        this->ptr = move(other.ptr);
        other.ptr = nullptr;
    }

    //* move constructor
    UniquePtr & operator=(UniquePtr && other) noexcept{
        if(this == &other) return *this;

        //* delete the previously owned data
        delete this->ptr;
        
        //* move data from othre / steal the pointer
        this->ptr = std::move(other.ptr);
        //* point other to nullprt so it dedstructs itself 
        other.ptr = nullptr;

        return *this;
    }

    T& operator*() const {
        return *ptr;
    }

    T* operator->() const {
        return ptr;
    }

};

int main(){

    UniquePtr<int> p1(new int(40));
    UniquePtr<int> p2;
    cout << "addr of p1 : " << *p1 << endl;
    p2 = std::move(p1);

    cout << "moved p2 "<< endl;
    cout << "addr of p2 : " << &p1 << endl;
    cout << "addr of p2 : " << &p2 << endl;

    return 0;
}