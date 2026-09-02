#include <iostream>

using namespace std;

class Test{
public:
    Test(){
        cout << "Constructor\n";
    }
    ~Test(){
        cout << "Destructor\n";
    }
};

int main(){
    cout << "Before\n";
    {
        Test t;
    }
    cout << "After\n";
    return 0;
}