#ifndef FINANCE_MONTE_CARLO_H
#define FINANCE_MONTE_CARLO_H

#include <vector>
#include <functional>
#include <random>
#include <thread>

class FinanceMonteCarlo {
public:
    FinanceMonteCarlo(int num_threads = std::thread::hardware_concurrency());

    // Option Pricing
    double price_european_option(double S, double K, double r, double sigma, double T, long long num_samples);

    // Greeks Calculation
    double calculate_delta(double S, double K, double r, double sigma, double T, long long num_samples);
    double calculate_gamma(double S, double K, double r, double sigma, double T, long long num_samples);
    double calculate_vega(double S, double K, double r, double sigma, double T, long long num_samples);
    double calculate_theta(double S, double K, double r, double sigma, double T, long long num_samples);
    double calculate_rho(double S, double K, double r, double sigma, double T, long long num_samples);

private:
    int num_threads_;
    std::vector<std::mt19937> random_engines_;

    // Helper methods
    double run_simulation_thread(std::function<double(std::mt19937&)> sim_func, long long samples_per_thread);
    std::vector<double> generate_correlated_normals(const std::vector<double>& correlations);
};

#endif // FINANCE_MONTE_CARLO_H
