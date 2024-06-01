
#include <iostream>
#include <cmath>
#include <cstdlib>

int k_Factorial_F(int k){
    int k_Factorial = 1;
    for(int i=1; i<=k ; i++){
        k_Factorial = k_Factorial * i;
    }
    return k_Factorial;
}

long double poisson(int k, double delta){
    int k_Factorial = k_Factorial_F(k);
    long double delta_pow_k = std::pow(delta, k);
    long double e_pow_minus_delta = std::exp(-delta);
    long double poi = (delta_pow_k/k_Factorial)*e_pow_minus_delta;
    return poi;
}
