#include <iostream>

using namespace std;

template<typename T>
class SharedPtr{

private:
    T *ptr;
    int *refCount;

    void release(){
        (*this->refCount)--;
        if(*this->refCount <= 0){
            delete this->refCount;
            delete this->ptr;
        }
    }

public:
    explicit SharedPtr(T* p = nullptr){
        this->ptr = p;
        this->refCount = new int(1);
    }

    ~SharedPtr() { release(); };

    //* copy Constructor
    SharedPtr(const SharedPtr& other){
        this->ptr = other.ptr;
        this->refCount = other.refCount;
        (*this->refCount)++;
    }

    //* copy Assignment
    SharedPtr& operator=(const SharedPtr& other){
        if(this == &other) return *this;

        release();

        this->ptr = other.ptr;
        this->refCount = other.refCount;
        (*this->refCount)++;

        return *this;
    }

    //* move Constructor
    SharedPtr(SharedPtr&& other) noexcept {
        this->ptr = other.ptr;
        this->refCount = other.refCount;

        other.ptr = nullptr;
        other.refCount = nullptr;
    }

    //* move Assignment
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if(this == &other) return *this;
        release();

        this->ptr = other.ptr;
        this->refCount = other.refCount;

        other.ptr = nullptr;
        other.refCount = nullptr;

        return *this;
    }

    T& operator*() const {
        return *ptr;
    }

    T* operator->() const {
        return ptr;
    }

    int use_count() const {
        return *refCount;
    }
};


int main(){
    SharedPtr<int> s1(new int(5));
    // cout << s1. << endl;
    return 0;
}