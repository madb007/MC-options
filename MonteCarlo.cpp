#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <limits>

class MonteCarloOptionPricer {
private:
    double S0;        // Initial stock price
    double K;         // Strike price
    double r;         // Risk-free rate
    double sigma;     // Volatility
    double T;         // Time to maturity
    int numSimulations;

    std::mt19937 gen;
    std::normal_distribution<> norm_dist;

public:
    MonteCarloOptionPricer(double S0, double K, double r, double sigma, double T, int numSimulations)
        : S0(S0), K(K), r(r), sigma(sigma), T(T), numSimulations(numSimulations),
          gen(std::random_device{}()), norm_dist(0.0, 1.0) {}

    double simulatePath() {
        double ST = S0 * exp((r - 0.5 * sigma * sigma) * T + sigma * sqrt(T) * norm_dist(gen));
        return std::max(ST - K, 0.0);
    }

    double priceEuropeanCall() {
        double sumPayoffs = 0.0;
        for (int i = 0; i < numSimulations; ++i) {
            sumPayoffs += simulatePath();
        }
        return exp(-r * T) * (sumPayoffs / numSimulations);
    }
}

int main() {
    double S0, K, r, sigma, T;
    int numSimulations;

    std::cout << "Enter initial stock price (S0): ";
    std::cin >> S0;

    std::cout << "Enter strike price (K): ";
    std::cin >> K;

    std::cout << "Enter risk-free rate (r): ";
    std::cin >> r;

    std::cout << "Enter volatility (sigma): ";
    std::cin >> sigma;

    std::cout << "Enter time to maturity in years (T): ";
    std::cin >> T;

    std::cout << "Enter number of simulations: ";
    std::cin >> numSimulations;

    // Clear any remaining characters in the input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    MonteCarloOptionPricer pricer(S0, K, r, sigma, T, numSimulations);
    double optionPrice = pricer.priceEuropeanCall();

    std::cout << "Estimated European Call Option Price: " << optionPrice << std::endl;

    return 0;


