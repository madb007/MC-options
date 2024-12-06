#define _USE_MATH_DEFINES
#include "FinanceMonteCarlo.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>
#include <thread>

FinanceMonteCarlo::FinanceMonteCarlo(int num_threads) 
    : num_threads_(num_threads) {
    // Initialize thread-local random number generators
    random_engines_.resize(num_threads_);
    std::random_device rd;
    for (auto& engine : random_engines_) {
        engine.seed(rd());
    }
}

double FinanceMonteCarlo::price_european_option(const OptionParams& p, bool is_call) {
    double drift = (p.r - 0.5 * p.v * p.v) * p.T;
    double diffusion = p.v * std::sqrt(p.T);
    
    // Load constants into SIMD registers
    __m256d _drift = _mm256_set1_pd(drift);
    __m256d _diffusion = _mm256_set1_pd(diffusion);
    __m256d _S = _mm256_set1_pd(p.S);
    __m256d _K = _mm256_set1_pd(p.K);
    __m256d _zero = _mm256_set1_pd(0.0);
    
    double total_sum = 0.0;
    
    #pragma omp parallel num_threads(num_threads_) reduction(+:total_sum)
    {
        std::normal_distribution<double> normal(0.0, 1.0);
        auto& engine = random_engines_[omp_get_thread_num()];
        
        // Pre-allocate arrays for SIMD operations
        std::vector<double> z_values(4);
        std::vector<double> payoffs(4);
        
        #pragma omp for schedule(dynamic, 1000)
        for (long long i = 0; i < p.numSamples; i += 4) {
            // Generate 4 random numbers
            for (int j = 0; j < 4; ++j) {
                double Z = normal(engine);
                z_values[j] = Z;
            }
            
            // Load random numbers into SIMD register
            __m256d _Z = _mm256_loadu_pd(z_values.data());
            
            // Calculate stock prices using SIMD
            __m256d _diffusionZ = _mm256_mul_pd(_diffusion, _Z);
            __m256d _total = _mm256_add_pd(_drift, _diffusionZ);
            __m256d _SForward = _mm256_mul_pd(_S, _mm256_exp_pd(_total));
            
            // Calculate payoffs
            __m256d _payoff;
            if (is_call) {
                _payoff = _mm256_max_pd(_mm256_sub_pd(_SForward, _K), _zero);
            } else {
                _payoff = _mm256_max_pd(_mm256_sub_pd(_K, _SForward), _zero);
            }
            
            // Store results and accumulate sum
            _mm256_storeu_pd(payoffs.data(), _payoff);
            for (int j = 0; j < 4; ++j) {
                total_sum += payoffs[j];
            }
        }
    }
    
    return std::exp(-p.r * p.T) * total_sum / p.numSamples;
}

// Global variables for Black-Scholes calculations
double d1, d2, Nd1, Nd2;

void FinanceMonteCarlo::calculate_d1_d2(const OptionParams& p) {
    //probability factor (D1) and a second probability factor (D2)
    d1 = (std::log(p.S / p.K) + (p.r + 0.5 * p.v * p.v) * p.T) / (p.v * std::sqrt(p.T));
    d2 = d1 - p.v * std::sqrt(p.T);

    // Calculate the cumulative normal distribution of d1 and d2
    Nd1 = 0.5 * (1 + std::erf(d1 / std::sqrt(2)));
    Nd2 = 0.5 * (1 + std::erf(d2 / std::sqrt(2)));
}

double FinanceMonteCarlo::black_scholes_price(const OptionParams& p, bool is_call) {
    //black-scholes formula price
    calculate_d1_d2(p);
      
    if (is_call) {
        return p.S * Nd1 - p.K * std::exp(-p.r * p.T) * Nd2;
    } else {
        return p.K * std::exp(-p.r * p.T) * (1 - Nd2) - p.S * (1 - Nd1);
    }
}

double FinanceMonteCarlo::black_scholes_delta(const OptionParams& p, bool is_call) {
    //Delta is first derivative of options price with respect to underlying price
    calculate_d1_d2(p);
    return is_call ? Nd1 : Nd1 - 1;
}

double FinanceMonteCarlo::black_scholes_gamma(const OptionParams& p) {
    //Gamma is second derivative of options price with respect to underlying price
    calculate_d1_d2(p);
    return std::exp(-0.5 * d1 * d1) / (p.S * p.v * std::sqrt(2 * M_PI * p.T));
}

double FinanceMonteCarlo::black_scholes_theta(const OptionParams& p, bool is_call) {
    //Theta is derivative of options price with respect to time 
    calculate_d1_d2(p);
    
    double theta = -p.S * p.v * std::exp(-0.5 * d1 * d1) / (2 * std::sqrt(2 * M_PI * p.T));
    
    if (is_call) {
        theta -= p.r * p.K * std::exp(-p.r * p.T) * Nd2;
    } else {
        theta += p.r * p.K * std::exp(-p.r * p.T) * (1 - Nd2);
    }
    //Get daily theta
    return theta/365;
}

double FinanceMonteCarlo::black_scholes_vega(const OptionParams& p) {
    //Vega is derivative of options price with respect to volatility
    calculate_d1_d2(p);
    //calculate vega and divide by 100 to account for decimal input
    return p.S * std::sqrt(p.T) * std::exp(-0.5 * d1 * d1) / (100 * std::sqrt(2 * M_PI));
}

double FinanceMonteCarlo::run_simulation_thread(std::function<double(std::mt19937&)> sim_func, long long samples_per_thread) {
    std::vector<std::thread> threads;
    std::vector<double> results(num_threads_);
    for (int i = 0; i < num_threads_; ++i) {
        //run simulations on different threads
        threads.emplace_back([&, i]() {
            double sum = 0.0;
            for (long long j = 0; j < samples_per_thread; ++j) {
                sum += sim_func(random_engines_[i]);
            }
            results[i] = sum;
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    return std::accumulate(results.begin(), results.end(), 0.0);
}
