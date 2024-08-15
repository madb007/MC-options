#define _USE_MATH_DEFINES
#include "FinanceMonteCarlo.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>
#include <thread>

FinanceMonteCarlo::FinanceMonteCarlo(int num_threads)
    //set up random engines for threads
    : num_threads_(num_threads) {
    random_engines_.resize(num_threads_);
    std::random_device rd;
    for (auto& engine : random_engines_) {
        engine.seed(rd());
    }
}

double FinanceMonteCarlo::price_european_option(const OptionParams& p, bool is_call) {
    auto sim_func = [&](std::mt19937& engine) {
        //Use Mersenne Twister pseudo random number generator
        double drift = (p.r - 0.5 * p.v * p.v) * p.T;
        double diffusion = p.v * std::sqrt(p.T);
        
        std::normal_distribution<double> normal(0.0, 1.0);
  
        double Z = normal(engine);
        double SForward = p.S * std::exp(drift + diffusion * Z);
        return is_call ? std::max(SForward - p.K, 0.0) : std::max(p.K - SForward, 0.0);
    };
    
    double sum = run_simulation_thread(sim_func, p.numSamples);
    return (std::exp(-p.r * p.T) * sum / p.numSamples) / num_threads_;
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
