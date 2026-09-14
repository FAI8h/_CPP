#include <iostream>

using namespace std;

template<typename T>
class SharedPtr{

private:
    T *ptr;
    int *refCount;

    void release(){
        if(this->refCount == nullptr) return; //* this prevents the seg* fault when releasing on a null refCount

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

//* test case
struct Widget {
    int id;
    Widget(int i) : id(i) { cout << "Widget " << id << " constructed" << endl; }
    ~Widget() { cout << "Widget " << id << " DESTROYED" << endl; }
};

int main(){
    cout << "Test 1 --- Basic shared count tracking  \n" << endl;
    SharedPtr<Widget> a(new Widget(1));
    {
        SharedPtr<Widget> b = a;
        cout << "count after copying a = " << a.use_count() << " (must be 2) "<< endl;
        cout << "b->id = " << b->id << endl;
        cout << "b's scope ends after scope Shared count should be back to 1"<< endl;
    }
    cout << "Shared count after scope ends = " << a.use_count() << endl;

    cout << "\n--- Test 2: copy assignment releases old object ---" << endl;
    SharedPtr<Widget> c(new Widget(2));
    c = a; // c should release Widget 2, then share Widget 1 with a
    cout << "count after c = a: " << a.use_count() << " (should be 2)" << endl;

    cout << "\n--- Test 3: move doesn't change count ---" << endl;
    SharedPtr<Widget> d = std::move(c);
    cout << "count after move: " << a.use_count() << " (should still be 2, c moved-from, doesn't count)" << endl;
    cout << "d use_count: " << d.use_count() << endl;

    cout << "\n--- end of main, remaining SharedPtrs (a, d) about to destruct ---" << endl;
    return 0;
}