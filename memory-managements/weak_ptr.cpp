#include <iostream>

using namespace std;

struct ControlBlock{
    int strongCount;
    int weakCount;
};
template <typename T> class WeakPtr;
template <typename T> class SharedPtr{
private:
    T *ptr;
    ControlBlock *ctrl;

    // Friends
    friend class WeakPtr<T>;

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

//* tests
struct Widget {
    int id;
    Widget(int i) : id(i) { cout << "Widget " << id << " constructed" << endl; }
    ~Widget() { cout << "Widget " << id << " DESTROYED" << endl; }
};

int main(){
    cout << "--- Test A: lock() while object alive ---" << endl;
    SharedPtr<Widget> sp(new Widget(1));
    WeakPtr<Widget> wp(sp);
    cout << "strong count before lock: " << sp.use_count() << endl;
    {
        SharedPtr<Widget> locked = wp.lock();
        cout << "lock() succeeded, id = " << locked->id << endl;
        cout << "strong count during lock: " << sp.use_count() << " (should be 2)" << endl;
    }
    cout << "strong count after locked goes out of scope: " << sp.use_count() << " (should be back to 1)" << endl;

    cout << "\n--- Test B: object dies, weak ptr survives, lock() fails safely ---" << endl;
    {
        SharedPtr<Widget> sp2(new Widget(2));
        WeakPtr<Widget> wp2(sp2);
        cout << "created sp2/wp2" << endl;
    } // sp2 dies here -> Widget 2 destroyed, but ctrl survives since wp2 still exists... 
      // wait wp2 also goes out of scope here at the same brace! both die together.
    
    cout << "\n--- Test C: weak ptr outlives shared ptr (separate scopes) ---" << endl;
    WeakPtr<Widget>* outerWeak = nullptr;
    {
        SharedPtr<Widget> sp3(new Widget(3));
        outerWeak = new WeakPtr<Widget>(sp3);
        cout << "sp3 alive, wp3 created" << endl;
    } // sp3 destructs here -> Widget 3 destroyed (strongCount hits 0), ctrl survives (weakCount still 1)
    
    cout << "sp3 out of scope now. attempting lock() on outerWeak..." << endl;
    SharedPtr<Widget> attempt = outerWeak->lock();
    if (attempt.use_count() == 0) {
        cout << "this check is wrong, use_count would crash on empty ptr, skip" << endl;
    }
    delete outerWeak; // cleans up weak, which should free ctrl now

    cout << "\n--- end of main ---" << endl;

    return 0;
}