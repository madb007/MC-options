// FinanceMonteCarlo.cpp
#include "FinanceMonteCarlo.h"
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace std;

FinanceMonteCarlo::FinanceMonteCarlo(int num_threads) 
    : num_threads_(num_threads) {
    random_engines_.resize(num_threads_);
    random_device rd;
    for (auto& engine : random_engines_) {
        engine.seed(rd());
    }
}

double FinanceMonteCarlo::price_european_option(double S, double K, double r, double sigma, double T, long long num_samples) {
    auto sim_func = [&](mt19937& engine) {
        normal_distribution<> normal(0, 1);  // Normal distribution used here
        double ST = S * exp((r - 0.5 * sigma * sigma) * T + sigma * sqrt(T) * normal(engine));
        return max(ST - K, 0.0);
    };
    
    double sum = run_simulation_thread(sim_func, num_samples);
    return exp(-r * T) * sum / num_samples;
}

double FinanceMonteCarlo::calculate_delta(double S, double K, double r, double sigma, double T, long long num_samples) {
    double h = 0.01 * S;  // Small change in stock price
    double V1 = price_european_option(S + h, K, r, sigma, T, num_samples);
    double V2 = price_european_option(S - h, K, r, sigma, T, num_samples);
    return (V1 - V2) / (2 * h);
}

double FinanceMonteCarlo::run_simulation_thread(function<double(mt19937&)> sim_func, long long samples_per_thread) {
    vector<thread> threads;
    vector<double> results(num_threads_);

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

vector<double> FinanceMonteCarlo::generate_correlated_normals(const vector<double>& correlations) {
    // This function generates correlated normal random variables
    
    int n = static_cast<int>(sqrt(correlations.size()));
    vector<double> result(n);
    normal_distribution<> normal(0, 1);  // Normal distribution used here
    
    for (int i = 0; i < n; ++i) {
        result[i] = normal(random_engines_[0]);
    }
    
    return result;
}
