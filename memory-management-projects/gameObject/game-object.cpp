#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <functional>

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

template<typename T, typename Alloc = MyAllocator<T>>
class MyVector {
private:
    Alloc allocator;
    T *data;
    size_t size;
    size_t capacity;
    using iterator = T *;

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
    MyVector(int capacity = 2, Alloc alloc = Alloc()) 
        : allocator(alloc), size(0) {
            if (capacity < 1){
                throw std::invalid_argument("capacity must be a positive");
            }
            this->capacity = capacity;
            data = allocator.allocate(capacity);
        };


    iterator begin() {return data;};
    iterator end() { return data + size; };

    T& operator[](int index){
        if(index < 0 || index >= size){
            throw std::out_of_range("Out of range access");
        }
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
        
        this->data = allocator.allocate(other.capacity);

        int i = 0;
        try{
            for (; i < other.size; i++){
                allocator.construct(&data[i], other.data[i]);
            }
        }catch(const std::exception& e){
            for (int j = 0; j < i; j++){
                allocator.destroy(&data[j]);
            }
            allocator.deallocate(data);
            std::cerr << e.what() << '\n';
            throw;
        }

        this->size = other.size;
        this->capacity = other.capacity;
    }

    //* copy assignment
    MyVector &operator=(const MyVector & other){
        /*
         * clarification on why we need to destory old object and deallocate the memory 
         * if the copy assignment is being called for already assigned object then we have to clear the object copy the new one entierly
         * now on why deallocate , Why not use the freed object that we just did , cause what if the object we are copying from has larger capacity and size (!* THINK ABOUT IT)  
        */
        if(this == &other) return *this;

        auto newArr = allocator.allocate(other.capacity);

        int i = 0;
        try{
            for (; i < other.size; i++){
                allocator.construct(&newArr[i], other.data[i]);
            }
        }
        catch(const std::exception& e){
            for (int j = 0; j < i; j++){
                allocator.destroy(&newArr[j]);
            }

            allocator.deallocate(newArr);
            std::cerr << e.what() << '\n';
            throw;
        }
        i = 0;
        for (; i < size; i++){
            allocator.destroy(&data[i]);
        }
        allocator.deallocate(data);

        this->data = newArr;
        this->capacity = other.capacity;
        this->size = other.size;

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

template<typename T>
struct ControlBlock{
    int strongCount;
    int weakCount;
    std::function<void(T *)> deleter;
};
template <typename T> class WeakPtr;
template<typename T> class EnableSharedFromThis;
template <typename T> class SharedPtr{
private:
    T *ptr;
    ControlBlock<T> *ctrl;

    // Friends
    friend class WeakPtr<T>;

    void release(){
        if(this->ctrl == nullptr) return; //* this prevents the seg* fault when releasing on a null refCount

        (this->ctrl->strongCount)--;
        if(this->ctrl->strongCount <= 0){

            bool shouldFreeCtrl = (this->ctrl->weakCount <= 0);
            
            std::function<void(T *)> deleter = this->ctrl->deleter;
            deleter(this->ptr);

            if(shouldFreeCtrl){
                delete this->ctrl;
            }

        }
    }

    SharedPtr(T* p, ControlBlock<T>* c){
        this->ptr = p;
        this->ctrl = c;

        (this->ctrl->strongCount)++;

    }

public:
    explicit SharedPtr(T* p = nullptr, std::function<void(T*)> deleter = [](T* ptr){delete ptr;}){
        this->ptr = p;
        this->ctrl = new ControlBlock<T>{1, 0, deleter};
        if constexpr (std::is_base_of<EnableSharedFromThis<T>, T>::value){
            p->weakSelf = *this;
        }
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
        ControlBlock<T>* ctrl;

        void release(){
            if(this->ctrl == nullptr) return;

            (this->ctrl->weakCount)--;
            if(this->ctrl->strongCount <= 0 && this->ctrl->weakCount <= 0){
                delete this->ctrl;
            }
        }

    public:
        
        WeakPtr(): ptr(nullptr), ctrl(nullptr) {}

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

        WeakPtr(const WeakPtr& other){
            this->ptr = other.ptr;
            this->ctrl = other.ctrl;

            this->ctrl->weakCount++;
        };

        WeakPtr& operator=(const WeakPtr& other){
            if(this == &other) return *this;

            release();

            this->ptr = other.ptr;
            this->ctrl = other.ctrl;
            this->ctrl->weakCount++;

            return *this;
        }

        int weak_count_debug() const { return this->ctrl ? this->ctrl->weakCount : 1; };
};

//* Memory pool
template<typename T>
class MemoryPool {
private:
    char *buffer;
    size_t capacity;
    void *freeListHead;

public:
    explicit MemoryPool(size_t numSlots){
        size_t slotSize = max(sizeof(T), sizeof(void *));

        this->capacity = slotSize * numSlots;
        this->buffer = static_cast<char *>(operator new(slotSize * numSlots));

        int i = 0;
        void **addr = nullptr;
        for (; i < numSlots - 1; i++){
            //addr
            addr = reinterpret_cast<void **>(buffer + i * slotSize);
            *(addr) = reinterpret_cast<void *>(buffer + (i + 1) * slotSize);
        }
        addr = reinterpret_cast<void **>(buffer + i * slotSize);
        *(addr) = nullptr;

        this->freeListHead = this->buffer;
    }

    ~MemoryPool(){
        ::operator delete(buffer);
    }

    T* allocate(){
        if(this->freeListHead == nullptr){
            throw std::out_of_range("pool exhausted");
        }

        void *slot = this->freeListHead;

        void * next = *reinterpret_cast<void **>(slot);

        this->freeListHead = next;

        return reinterpret_cast<T *>(slot);
    }

    void deallocate(T* ptr){
        char *p = reinterpret_cast<char *>(ptr);
        if (this->buffer > p || this->buffer + this->capacity <= p){
            throw std::range_error("bad pointer access");
        }

        *reinterpret_cast<void **>(ptr) = this->freeListHead;
        this->freeListHead = ptr;
    }
};

//* Arena Allocator
class Arena{
private:
    char *buffer; // raw memory block
    size_t capacity; // total size of the block
    size_t offset; // how much has been used

public:
    explicit Arena(size_t size){
        this->buffer = static_cast<char* >(::operator new(size));
        this->capacity = size;
        this->offset = 0;
    }

    ~Arena(){
        ::operator delete(buffer);
    }

    template<typename T>
    T* allocate(size_t count = 1){
        size_t bytesNeeded = count * sizeof(T);
        size_t alignment = alignof(T);

        size_t padding = alignment - (offset % alignment);

        size_t localOffset = this->offset + padding;

        if(alignment == padding) localOffset = this->offset;

        if(bytesNeeded + localOffset > capacity){
            throw std::out_of_range("insufficient capaity");
        }
        T* addr = reinterpret_cast<T *>(buffer + localOffset);

        offset = localOffset + bytesNeeded;
        return addr;
    }
};

//* Arena Allocator Adapter
template<typename T>
class ArenaAllocatorAdapter{
private:
    Arena &arena;
public:
    explicit ArenaAllocatorAdapter(Arena &a) : arena(a) {}

    T* allocate(size_t count){
        return arena.allocate<T>(count);
    };

    template<typename... Args>
    void construct(T* ptr, Args &&... args){
        new(ptr) T(std::forward<Args>(args)...);
    }

    void destroy(T* ptr){
        ptr->~T();
    }

    void deallocate(T* ptr){

    }
};

//* --------------------------------game object---------------------------------------
template<typename T>
class EnableSharedFromThis{
    friend class SharedPtr<T>;
protected:
    WeakPtr<T> weakSelf;
public:
    SharedPtr<T> shared_from_this(){
        return weakSelf.lock();
    }
};
class GameObject : public EnableSharedFromThis<GameObject>{
private:
    string name;
    MyVector<SharedPtr<GameObject>> children;
    WeakPtr<GameObject> parent;
public:
    GameObject(string name) : name(name){
        cout << name << " Constructed\n";
    }
    
    ~GameObject(){
        cout << name << " Destructed\n";
    }

    void addChild(const SharedPtr<GameObject>& self, const SharedPtr<GameObject>& child){
        child->parent = self;

        this->children.push_back(child);
    };

    string getName() const { return this->name; };
    WeakPtr<GameObject> getParent() const { return this->parent; };
};

//? --------------------- Global Functions -----------------------------
template <typename T, typename... Args>
SharedPtr<T> makeFromPool(MemoryPool<T> &pool, Args &&...args)
{
    T *slot = pool.allocate();
    
    new (slot) T(std::forward<Args>(args)...);

    return SharedPtr<T>(
        slot,
        [&pool](T* p){
            p->~T();
            pool.deallocate(p);
        });
}

int main(){
    // Variant 1: plain MyVector, default MyAllocator
    MyVector<int> plainVec;
    plainVec.push_back(1);
    plainVec.push_back(2);
    plainVec.push_back(3);
    plainVec.push_back(4); // forces grow()
    cout << "plainVec: ";
    for (auto& v : plainVec) cout << v << " ";
    cout << "\n";
 
    // Variant 2: Arena-backed MyVector
    Arena arena(4096);
    ArenaAllocatorAdapter<int> arenaAlloc(arena);
    MyVector<int, ArenaAllocatorAdapter<int>> arenaVec(2, arenaAlloc);
    arenaVec.push_back(10);
    arenaVec.push_back(20);
    arenaVec.push_back(30); // forces grow() -> old block leaked in arena, expected
    cout << "arenaVec: ";
    for (auto& v : arenaVec) cout << v << " ";
    cout << "\n";
 
    MemoryPool<GameObject> pool(8);
 
    SharedPtr<GameObject> root = makeFromPool(pool, "root");
    SharedPtr<GameObject> childA = makeFromPool(pool, "childA");
    SharedPtr<GameObject> childB = makeFromPool(pool, "childB");
 
    root->addChild(root, childA);
    root->addChild(root, childB);
 
    cout << "root use_count: " << root.use_count() << "\n";
    cout << "childA use_count: " << childA.use_count() << "\n";
 
    WeakPtr<GameObject> parentOfA = childA->getParent();
    SharedPtr<GameObject> lockedParent = parentOfA.lock();
    cout << "childA's parent (locked): " << lockedParent->getName() << "\n";
 
    cout << "--- end of scope, destructors fire ---\n";
    return 0;
}