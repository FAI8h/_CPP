#include <iostream>
#include <memory>

using namespace  std;

template<typename T>

class MyVector {
private:
    unique_ptr<T[]> data;
    int size;
    int capacity;
public:
    MyVector(int size = 0, int capacity = 2) 
        : size(size), capacity(capacity){
            data = make_unique<T[]>(capacity);
        };


    T& operator[](int index){
        return data[index];
    }

    void push_back(T val){
        if(size == capacity){
            capacity = capacity * 2;
            auto newArr = make_unique<T[]>(capacity);

            for (int i = 0; i < size; i++){
                newArr[i] = move(data[i]);
            }
            data = move(newArr);
        }
        data[size] = val;
        size++;
    }
};

int main () {

    // MyVector<int> a;
    // a.push_back(1);
    // a.push_back(2);
    // a.push_back(3);

    // for (int i = 0; i < 3; i++){
    //     cout << "data a : " << a[i] << " addr a : " << &a[i] << endl;
    // }
    
    MyVector<string> b;
    b.push_back("alice");
    b.push_back("bob");
    b.push_back("john");

    for (int i = 0; i < 4; i++){
        cout << "size : " << sizeof(b[i])<<endl;
        cout << "data b : " << b[i] << " addr b : " << static_cast<void *>(&b[i]) << endl;
    }

    return 0;
}