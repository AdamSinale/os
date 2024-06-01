
#include "Poisson.cpp"
#include <iostream>
#include <cmath>
#include <cstdlib>

int main(){
    std::cout << "P(X=" << 1 << ") = " << poisson(1,2) << std::endl;
    std::cout << "P(X=" << 10 << ") = " << poisson(10,2) << std::endl;
    std::cout << "P(X=" << 2 << ") = " << poisson(2,2) << std::endl;
    std::cout << "P(X=" << 3 << ") = " << poisson(3,3) << std::endl;
    std::cout << "P(X=" << 3 << ") = " << poisson(3,100) << std::endl;
    return 0;
}