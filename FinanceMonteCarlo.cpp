// FinanceMonteCarlo.cpp
#define _USE_MATH_DEFINES


#include "FinanceMonteCarlo.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>
#include <thread>


FinanceMonteCarlo::FinanceMonteCarlo(int num_threads) 
    : num_threads_(num_threads) {
    random_engines_.resize(num_threads_);
    std::random_device rd;
    for (auto& engine : random_engines_) {
        engine.seed(rd());
    }
}

double FinanceMonteCarlo::price_european_call_option(const OptionParams& p) {
    auto sim_func = [&](std::mt19937& engine) {
        double drift = (p.r - 0.5 * p.v * p.v) * p.T;
        double diffusion = p.v * std::sqrt(p.T);
        
        std::normal_distribution<double> normal(0.0,1.0);
  
        double Z = normal(engine);
        double SForward = p.S * std::exp(drift + diffusion * Z);
        return std::max(SForward - p.K, 0.0);
    };
    
    double sum = run_simulation_thread(sim_func, p.numSamples);
    return (std::exp(-p.r * p.T) * sum / p.numSamples)/num_threads_;
}

double FinanceMonteCarlo::price_european_put_option(const OptionParams& p) {
    auto sim_func = [&](std::mt19937& engine) {
        double drift = (p.r - 0.5 * p.v * p.v) * p.T;
        double diffusion = p.v * std::sqrt(p.T);
        
        std::normal_distribution<double> normal(0.0,1.0);
  
        double Z = normal(engine);
        double SForward = p.S * std::exp(drift + diffusion * Z);
        return std::max(p.K-SForward, 0.0);
    };
    
    double sum = run_simulation_thread(sim_func, p.numSamples);
    return (std::exp(-p.r * p.T) * sum / p.numSamples)/num_threads_;
}

double FinanceMonteCarlo::calculate_delta(const OptionParams& p, bool is_call) {
    double h = 0.01 * p.S;  // Small change in stock price
    OptionParams p_up = p;
    OptionParams p_down = p;
    p_up.S += h;
    p_down.S -= h;

    double V1 = is_call ? price_european_call_option(p_up) : price_european_put_option(p_up);
    double V2 = is_call ? price_european_call_option(p_down) : price_european_put_option(p_down);
    return (V1 - V2) / (2 * h);
}

double FinanceMonteCarlo::calculate_gamma(const OptionParams& p) {
    double h = 0.03 * p.S;  // Small change in stock price
    OptionParams p_up = p;
    OptionParams p_down = p;
    p_up.S += h;
    p_down.S -= h;
    
    double delta_up = calculate_delta(p_up,true);
    double delta_down = calculate_delta(p_down,true);
    return (delta_up-delta_down) / (2 * h);
}

double FinanceMonteCarlo::calculate_theta(const OptionParams& p, bool is_call) {
    double h = 0.05;  // Small change in time
    OptionParams p_forward = p;
    p_forward.T -= h;

    double V1 = is_call ? price_european_call_option(p) : price_european_put_option(p);
    double V2 = is_call ? price_european_call_option(p_forward) : price_european_put_option(p_forward);
    return -(V2 - V1) / h;
}

double FinanceMonteCarlo::calculate_vega(const OptionParams& p) {
    double h = 0.05;  // Small change in volatility
    OptionParams p_up = p;
    p_up.v += h;

    double V1 = price_european_call_option(p);
    double V2 = price_european_call_option(p_up);
    return (V2 - V1) / h;
}

double FinanceMonteCarlo::run_simulation_thread(std::function<double(std::mt19937&)> sim_func, long long samples_per_thread) {
  std::vector<std::thread> threads;
  std::vector<double> results(num_threads_);

    for (int i = 0; i < num_threads_; ++i) {
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

    return accumulate(results.begin(), results.end(), 0.0);
}

