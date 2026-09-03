#include <iostream>
#include <memory>

using namespace std;

class MyVector {
private:
    unique_ptr<int[]> data;
    int size;
    int capacity;
public:
    MyVector(int size = 0, int capacity = 2) 
        : size(size), capacity(capacity){
            data = make_unique<int[]>(capacity);
        };

    //* this is copy constructor
    MyVector(const MyVector &other)
        : size(other.size), capacity(other.capacity){
            data = make_unique<int[]>(other.capacity);

        for(int i = 0; i < other.size; i++){
            data[i] = other.data[i];
        }
    }

    //* this is copy assignment
    MyVector &operator=(const MyVector &other) {
        if(this == &other) return *this;

        this->capacity = other.capacity;
        this->size = other.size;
        data = make_unique<int[]>(other.capacity);

        for (int i = 0; i < other.size; i++){
            data[i] = other.data[i];
        }

        return *this;
    }
    int& operator[](int index){
        return data[index];
    }

    void push_back(int val){
        if(size == capacity){
            capacity = capacity * 2;
            auto newArr = make_unique<int[]>(capacity);

            for (int i = 0; i < size; i++){
                newArr[i] = data[i];
            }
            data = move(newArr);
        }
        data[size] = val;
        size++;
    }

};

int main(){
    MyVector a;
    a.push_back(1);

    MyVector b = a;

    for (int i = 0; i < 5; i++){
        cout << "data a : "<<a[i] << " addr a : "<< &a[i] << "data b : "<<b[i] << " addr b : "<< &b[i] <<endl;
    }

        return 0;
}