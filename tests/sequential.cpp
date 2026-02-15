// sequential_large.cpp
#include <iostream>
#include <cstring>

int main() {
    const int SIZE = 1000000;  // 1 million
    int* arr = new int[SIZE];
    
    // Initialize
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    // Sequential access - very cache friendly
    long long sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    
    std::cout << "Sum: " << sum << std::endl;
    delete[] arr;
    return 0;
}