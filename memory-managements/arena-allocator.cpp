#include <iostream>

using namespace std;

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

        offset += localOffset + bytesNeeded;
        return addr;
    }
};

int main(){

    Arena arena(64);

    int *a = arena.allocate<int>();

    *a = 40;
    cout << "a = " << *a << " at " << (void *)a << endl;
    
    double* b = arena.allocate<double>();
    *b = 3.14;
    cout << "b = " << *b << " at " << (void *)b << endl;

    cout << "gap B/w a & b = " << (char *)b - (char *)a << " bytes" << endl;

    //* req more than remaning bytes
    try
    {
        int *many = arena.allocate<int>(100);
        cout << "should not print this" << endl;
    }
    catch(const std::exception& e)
    {
        cout << "correctly threw : "<< e.what() << '\n';
    }
    

    return 0;
}