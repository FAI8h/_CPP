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
            data = std::move(newArr);
        }
        data[size] = val;
        size++;
    }

};

int main(){
    MyVector a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(4);

    MyVector b = std::move(a);

    for (int i = 0; i < 4; i++){
        cout << "data b : "<<b[i] << " addr b : "<< &b[i] <<endl;
    }

        return 0;
}