#include <iostream>
#include <cstring>

int main() {
    const int SIZE = 100000000;  // 100 million instead of 1 million
    int* arr = new int[SIZE];
    
    // Initialize
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    // Sequential access
    long long sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    
    std::cout << "Sum: " << sum << std::endl;
    delete[] arr;
    return 0;
}