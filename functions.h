#include <vector>
#include <random>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

double normalCDF(double x);
double bsOptionPrice(bool call, double s, double k, double r, double t, double sigma);
double binomialOptionPrice(bool call, double s, double k, double r, double t, double sigma, int n);
double calculateAverage(const std::vector<double>& values);
double impliedVolatility(double S, double K, double T, double r, double marketPrice, double tol, int maxIter);
struct Asset {
    const char* name;
    double price;
    double drift;
    double volatility;
};
struct Option {
    Asset stock;
    bool call;
    double premium;
    double strike;
    double t;

};
class WeinerProcessSimulator{
private:
    double price;
    double mu;
    double sigma;
    double dt;
    std::mt19937 gen;

    double generateNormal(double mean, double stddev);

public:
    WeinerProcessSimulator(double initialPrice,double drift, double volatility, double timeStep);
    
    void simulateStep(bool show);
    void runSimulation(int n, int delay_ms, bool show);
    double getPrice() {return price;}
};

class MonteCarloSimulation {
private:
    int iterations;
    double duration;
    double increment;
    Asset stock;

public: 
    MonteCarloSimulation(int iter, double durat, double dt, Asset stock);
    double simulateOption(Option option);

};
#endif // FUNCTIONS_H