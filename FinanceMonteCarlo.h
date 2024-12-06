#pragma once
#include <random>
#include <vector>
#include <immintrin.h>
#include <omp.h>

struct OptionParams {
    double S;          // Spot price
    double K;          // Strike price
    double T;          // Time to maturity
    double r;          // Risk-free rate
    double v;          // Volatility
    long long numSamples;  // Number of Monte Carlo samples
};

class FinanceMonteCarlo {
public:
    FinanceMonteCarlo(int num_threads);
    
    // Monte Carlo method with OpenMP and SIMD optimizations
    double price_european_option(const OptionParams& p, bool is_call);
    
    // Black-Scholes methods (unchanged)
    double black_scholes_price(const OptionParams& p, bool is_call);
    double black_scholes_delta(const OptionParams& p, bool is_call);
    double black_scholes_gamma(const OptionParams& p);
    double black_scholes_theta(const OptionParams& p, bool is_call);
    double black_scholes_vega(const OptionParams& p);

private:
    int num_threads_;
    std::vector<std::mt19937> random_engines_;
    double d1, d2, Nd1, Nd2;
    
    void calculate_d1_d2(const OptionParams& p);
};

// Global variables for Black-Scholes calculations
extern double d1, d2, Nd1, Nd2;
