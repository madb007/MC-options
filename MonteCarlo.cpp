#include <iostream>
#include <cmath>
#include <random>
#include <vector>

class MonteCarloOptionPricer {
private:
    double S0;        // Initial stock price
    double K;         // Strike price
    double r;         // Risk-free rate
    double sigma;     // Volatility
    double T;         // Time to maturity
    int numSimulations;
}
