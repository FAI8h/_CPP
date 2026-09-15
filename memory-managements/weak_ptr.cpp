#include <iostream>
#include <memory>

using namespace std;

struct ControlBlock{
    int strongCount;
    int weakCount;
};

template <typename T>
class SharedPtr{
private:
    T *ptr;
    ControlBlock *ctrl;

    // Friends
    friend class weak_ptr<T>;

    void release(){
        if(this->ctrl == nullptr) return; //* this prevents the seg* fault when releasing on a null refCount

        (this->ctrl->strongCount)--;
        if(this->ctrl->strongCount <= 0){
            delete this->ptr;
        }
        
        if(this->ctrl->weakCount <= 0 && this->ctrl->strongCount <= 0){
            delete this->ctrl;
        }
    }

    SharedPtr(T* p, ControlBlock* c){
        this->ptr = p;
        this->ctrl = c;

        (this->ctrl->strongCount)++;

    }

public:
    explicit SharedPtr(T* p = nullptr){
        this->ptr = p;
        this->ctrl = new ControlBlock{1, 0};
    }

    ~SharedPtr() { release(); };

    //* copy Constructor
    SharedPtr(const SharedPtr& other){
        this->ptr = other.ptr;
        this->ctrl = other.ctrl;
        (this->ctrl->strongCount)++;
    }

    //* copy Assignment
    SharedPtr& operator=(const SharedPtr& other){
        if(this == &other) return *this;

        release();

        this->ptr = other.ptr;
        this->ctrl = other.ctrl;
        (this->ctrl->strongCount)++;

        return *this;
    }

    //* move Constructor
    SharedPtr(SharedPtr&& other) noexcept {
        this->ptr = other.ptr;
        this->ctrl = other.ctrl;

        other.ptr = nullptr;
        other.ctrl = nullptr;
    }

    //* move Assignment
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if(this == &other) return *this;
        release();

        this->ptr = other.ptr;
        this->ctrl = other.ctrl;

        other.ptr = nullptr;
        other.ctrl = nullptr;

        return *this;
    }

    T& operator*() const {
        return *ptr;
    }

    T* operator->() const {
        return ptr;
    }

    int use_count() const {
        return this->ctrl->strongCount;
    }
};

template <typename T>
class WeakPtr{
    private:
        T *ptr;
        ControlBlock* ctrl;

        void release(){
            if(this->ctrl == nullptr) return;

            (this->ctrl->weakCount)--;
            if(this->ctrl->strongCount <= 0 && this->ctrl->weakCount <= 0){
                delete this->ctrl;
            }
        }

    public:
        WeakPtr(const SharedPtr<T> & sp){
            this->ptr = sp.ptr;
            this->ctrl = sp.ctrl;
            this->ctrl->weakCount++;
        };

        ~WeakPtr(){
            release();
        }

        SharedPtr<T> lock() const {
            if(this->ctrl->strongCount <= 0) return SharedPtr<T>();

            return SharedPtr<T>(this->ptr, this->ctrl);
        }
};

int main(){

    SharedPtr<int> a(new int(10));

    cout << a.use_count() << endl;

    return 0;
}