#include <iostream>

using namespace std;

template<typename T>
class SharedPtr{

private:
    T *ptr;
    int *refCount;

public:
    explicit SharedPtr(T* p = nullptr){
        this->ptr = p;
        this->refCount = new int(1);
    }

    ~SharedPtr(){
        (*this->refCount)--;
        
        if(*refCount <= 0){
            delete this->refCount;
            delete this->ptr;
        }
    }

};

int main(){
    SharedPtr<int> s1(new int(5));
    // cout << s1. << endl;
    return 0;
}