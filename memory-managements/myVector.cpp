#include <iostream>

using namespace std;

class MyVector{
private:
    int *data;
    int size;
    
    public:
    int capacity;
    MyVector(int size = 0, int capacity = 2){
        this->size = size;
        this->capacity = capacity;
        data = new int[capacity];
    };

    MyVector(const MyVector &other)
        : size(other.size), capacity(other.capacity){
            data = new int[other.capacity];

        for(int i = 0; i < other.size; i++){
            data[i] = other.data[i];
        }
    }

    MyVector(MyVector &&other){
        this->data = other.data;

        other.data = nullptr;

        delete[] other.data;
    }

    MyVector &operator=(const MyVector &other) {
        if(this == &other) return *this;
        delete[] data;

        this->capacity = other.capacity;
        this->size = other.size;
        data = new int[other.capacity];

        for (int i = 0; i < other.size; i++){
            data[i] = other.data[i];
        }

        return *this;
    }

    void push_back(int val){
        if(size == capacity){
            capacity = capacity * 2;
            int *newArr = new int[capacity];

            for (int i = 0; i < size; i++){
                newArr[i] = data[i];
            }

            delete[] data;

            data = newArr;
        }
        data[size] = val;
        size++;
    }
    int& operator[](int index){
        return data[index];
    }


    ~MyVector(){
        delete[] data;
    };
};

MyVector createVector(){
    MyVector temp;
    temp.push_back(10);
    temp.push_back(20);

    return temp;
}

int main(){


    MyVector a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(4);
    a.push_back(5);


    a = a;

    for (int i = 0; i < 5; i++){
        // cout << "data a : "<<a[i] << " addr a : "<< &a[i] << " data b : "<< b[i] << " addr b : " << &b[i]<< endl;
        cout << "data a : "<<a[i] << " addr a : "<< &a[i] << endl;
    }

    return 0;
}