#include <iostream>
#include <memory>

using namespace std;

class Myvector {
private:
    unique_ptr<int[]> data;
    int size;
    int capacity;
public:

};

int main(){
    Myvector a;
    
    Myvector b = a;

    return 0;
}