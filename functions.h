#include <vector>
#include <random>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

double normalCDF(double x);
double bsOptionPrice(bool call, double s, double k, double r, double t, double sigma);
double binomialOptionPrice(bool call, double s, double k, double r, double t, double sigma, int n, bool show_steps);
double calculateAverage(const std::vector<double>& values);
double impliedVolatility(double S, double K, double T, double r, double marketPrice, double tol, int maxIter);

struct Asset {
    std::string name;
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
    bool keep_going;

    double generateNormal(double mean, double stddev);

public:
    WeinerProcessSimulator(double initialPrice,double drift, double volatility, double timeStep, bool keep_going);
    
    void simulateStep(bool show);
    void runSimulation(int n, int delay_ms, bool show);
    double getPrice() {return price;}
};

class MonteCarloSimulation {
private:
    int iterations;
    int duration;
    double increment;
    Asset stock;
    bool show;
    bool keep_going;
    

public: 
    MonteCarloSimulation(int iter, int durat, double dt, Asset stock, bool show);
    double estimateOption(Option option);
};
#endif // 