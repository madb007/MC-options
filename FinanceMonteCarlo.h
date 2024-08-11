#ifndef FINANCE_MONTE_CARLO_H
#define FINANCE_MONTE_CARLO_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <random>
#include <functional>
#include <thread>

struct OptionParams {
    double S;
    double K;
    double r;
    double v;
    double T;
    long long numSamples;
};

class FinanceMonteCarlo {
public:
    explicit FinanceMonteCarlo(int num_threads);

    double price_european_call_option(const OptionParams& p);
    double price_european_put_option(const OptionParams& p);
    double calculate_delta(const OptionParams& p, bool is_call);
    double calculate_gamma(const OptionParams& p);
    double calculate_theta(const OptionParams& p, bool is_call);
    double calculate_vega(const OptionParams& p);

private:
    int num_threads_;
    std::vector<std::mt19937> random_engines_;

    double run_simulation_thread(std::function<double(std::mt19937&)> sim_func, long long samples_per_thread);
};

#endif 

