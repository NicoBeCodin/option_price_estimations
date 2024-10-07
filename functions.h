#include <vector>
#include <random>
#include <atomic>
#include <mutex>
#include <thread>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

double normalCDF(double x);
double bsOptionPrice(bool call, double s, double k, double r, double t, double sigma);
double binomialOptionPrice(bool call, double s, double k, double r, double t, double sigma, int n, bool show_steps);
double calculateAverage(const std::vector<double> &values);
double impliedVolatility(double S, double K, double T, double r, double marketPrice, double tol, int maxIter);

extern std::atomic<bool> sim_stop;
extern std::atomic<bool> sim_running;
extern std::atomic<bool> sim_paused;
extern std::atomic<double> current_price;

extern std::mutex sim_mutex;
extern std::thread simulation_thread;

extern std::atomic<bool> mcs_running;
extern std::atomic<bool> mcs_stop;
extern std::atomic<double> mcs_approx_price;
extern std::atomic<double> mcs_progress;
extern std::atomic<bool> mcs_finish;

extern std::mutex mcs_mutex;
extern std::thread mcs_thread;

struct Asset
{
    std::string name;
    double price;
    double drift;
    double volatility;
    double interest_rate;
};
struct Option
{
    Asset stock;
    bool call;
    double premium;
    double strike;
    double t;
};
class WeinerProcessSimulator
{
private:
    double price;
    double mu;
    double sigma;
    double dt;
    std::mt19937 gen;
    bool keep_going;

    double generateNormal(double mean, double stddev);

public:
    WeinerProcessSimulator(double initialPrice, double drift, double volatility, double timeStep, bool keep_going);

    void simulateStep(bool show);
    void runSimulation(int n, int delay_ms, bool show);
    double getPrice() { return price; }
};

void runSimulationThread(int ms_delay, WeinerProcessSimulator &wps, bool sim_show_steps);
void stopCurrentSimulation();

class MonteCarloSimulation
{

public:
    int iterations;
    int duration;
    Asset stock;
    double increment;
    bool show;
    MonteCarloSimulation(int iter, int durat, double dt, Asset stock, bool show);
    double estimateOption(Option option);
};

void runMonteCarloThread(MonteCarloSimulation &mcs, Option &option);
void stopMonteCarloThread();

double runMonteCarloSim(int start, int end, MonteCarloSimulation mcs, Option option);
double runMonteCarloMultiThreading(int n_threads, MonteCarloSimulation mcs, Option option);

#endif //
