
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>
#include <vector>
#include <algorithm>
#include "functions.h"

// For Weiner Process
std::atomic<bool> sim_stop(false);
std::atomic<bool> sim_running(false);
std::atomic<bool> sim_paused(false);
std::atomic<double> current_price(0.0);

std::mutex sim_mutex;
std::thread simulation_thread;

// For Montecarlo simulation
std::atomic<bool> mcs_running(false);
std::atomic<bool> mcs_stop(false);
std::atomic<double> mcs_approx_price(0.0);
std::atomic<double> mcs_progress(0.0);

std::mutex mcs_mutex;
std::thread mcs_thread;

/*
Project structure:
MonteCarlo manages the WeinerProcesses
*/

#define PRINT_STEP(show, ...)    \
    do                           \
    {                            \
        if (show)                \
            printf(__VA_ARGS__); \
    } while (0)

double normalCDF(double x)
{
    return 0.5 * erfc(-x * M_SQRT1_2);
}

double bsOptionPrice(bool call, double s, double k, double r, double t, double sigma)
{
    double d1 = (log(s / k) + (r + 0.5 * sigma * sigma) * t) / (sigma * sqrt(t));
    double d2 = d1 - sigma * sqrt(t);

    // printf("d1: %f d2: %f \n", d1, d2);

    if (call)
    {
        return (normalCDF(d1) * s - normalCDF(d2) * k * (exp(-r * t)));
    }
    else
    {
        return (normalCDF(-d2) * k * exp(-r * t) - normalCDF(-d1) * s);
    }
}

double binomialOptionPrice(bool call, double s, double k, double r, double t, double sigma, int n, bool show_steps)
{
    double time_step = t / (double)n;
    double up_factor = exp(sigma * sqrt(time_step));
    double down_factor = 1.0 / up_factor;
    double risk_neutral_prob = (exp(r * time_step) - down_factor) / (up_factor - down_factor);
    double discount_factor = exp(-r * time_step);

    PRINT_STEP(show_steps, "Up_factor %f, Down_factor %f\n", up_factor, down_factor);

    // Init 2D array for stock price and option price
    std::vector<std::vector<double>> stock_prices(n + 1, std::vector<double>(n + 1));
    std::vector<std::vector<double>> option_values(n + 1, std::vector<double>(n + 1));
    // All possible stocks for n length of simulation (binomial tree, nth step has n+1 possible outputs (See pascal triangle))
    PRINT_STEP(show_steps, "stock price tree generation");
    for (int i = 0; i <= n; ++i)
    {
        for (int j = 0; j <= i; ++j)
        {
            stock_prices[i][j] = s * pow(up_factor, j) * pow(down_factor, i - j); // S0 * u^j * d^(i-j)
            PRINT_STEP(show_steps, "i: %d, j: %d stock_price: %f\n", i, j, stock_prices[i][j]);
        }
    }

    // Initialize option value from expiration
    for (int j = 0; j <= n; ++j)
    {
        if (call)
        {

            option_values[n][j] = std::max(stock_prices[n][j] - k, 0.0);
        }
        else
        {
            option_values[n][j] = std::max(k - stock_prices[n][j], 0.0);
        }
        PRINT_STEP(show_steps, "j: %d option_value: %f\n", j, option_values[n][j]);
    }
    // Work backwards
    PRINT_STEP(show_steps, "Backward processing\n");

    for (int i = n - 1; i >= 0; --i)
    {
        for (int j = i; j >= 0; --j)
        {
            PRINT_STEP(show_steps, "i: %d j: %d\n", i, j);
            double hold_value = discount_factor * (risk_neutral_prob * option_values[i + 1][j + 1] + (1.0 - risk_neutral_prob) * option_values[i + 1][j]);
            double exercise_value = 0.0;

            if (call)
            {
                exercise_value = std::max(stock_prices[i][j] - k, 0.0);
            }
            else
            {
                exercise_value = std::max(k - stock_prices[i][j], 0.0);
            }

            option_values[i][j] = std::max(hold_value, exercise_value);

            PRINT_STEP(show_steps, "hold_value %f, exercise_value %f, option_value %f, stock_value %f\n\n", hold_value, exercise_value, option_values[i][j], stock_prices[i][j]);
        }
    }
    PRINT_STEP(show_steps, "final price: %f\n\n", option_values[0][0]);
    return option_values[0][0];
}

double calculateAverage(const std::vector<double> &values)
{
    if (values.empty())
    {
        throw std::invalid_argument("Empty list, can't calculate average");
    }
    double sum = 0.0;
    for (const double &val : values)
    {
        sum += val;
    }
    return sum / values.size();
}

// refine this func
double impliedVolatility(double S, double K, double T, double r, double marketPrice, double tol = 1e-6, int maxIter = 1000)
{
    double lowVol = 0.0;
    double highVol = 5.0; // Start with a high guess for volatility
    double midVol = 0.0;

    for (int i = 0; i < maxIter; ++i)
    {
        midVol = (lowVol + highVol) / 2.0;
        double price = bsOptionPrice(true, S, K, T, r, midVol);

        if (fabs(price - marketPrice) < tol)
        {
            return midVol; // Solution found
        }

        if (price > marketPrice)
        {
            highVol = midVol; // Volatility too high, reduce it
        }
        else
        {
            lowVol = midVol; // Volatility too low, increase it
        }
    }

    // If it doesn't converge, return the best guess
    return midVol;
}
WeinerProcessSimulator::WeinerProcessSimulator(double initialPrice, double drift, double volatility, double timeStep, bool loop) : price(initialPrice), mu(drift), sigma(volatility), dt(timeStep), keep_going(loop) {}

// actually geometric brownian motion
void WeinerProcessSimulator::simulateStep(bool show)
{

    // Geometric brownian motion
    double dW = sqrt(dt) * generateNormal(0.0, 1.0);
    // double increment = mu*price*dt + sigma*price*dW;
    // price += increment;

    double new_price = price * exp((mu - 0.5 * pow(sigma, 2.0)) * dt + sigma * dW);

    PRINT_STEP(show, "price:%f, new_price: %f\n", price, new_price);
    price = new_price;
}

void WeinerProcessSimulator::runSimulation(int n, int delay_ms, bool show)
{
    if (n == -1)
    {
        int j = 0;
        while (keep_going)
        {
            PRINT_STEP(show, "step %d", j++);
            simulateStep(show);
            if (delay_ms != 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            }
        }
    }
    for (int i = 0; i < n; i++)
    {
        PRINT_STEP(show, "step %d\n", i);
        simulateStep(show);
        if (delay_ms != 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    }
}

double WeinerProcessSimulator::generateNormal(double mean, double stddev)
{
    static std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne twister
    std::normal_distribution<> d(mean, stddev);
    return d(gen);
}

void runSimulationThread(int ms_delay, WeinerProcessSimulator &wps, bool sim_show_steps)
{
    sim_running = true;
    while (!sim_stop)
    {
        if (sim_paused)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(sim_mutex);
            wps.simulateStep(sim_show_steps);
            current_price = wps.getPrice();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_delay));
    }
    sim_running = false;
}
void stopCurrentSimulation()
{
    if (sim_running)
    {
        sim_stop = true;
        if (simulation_thread.joinable())
        {
            simulation_thread.join();
        }
        sim_stop = false;
        sim_running = false;
        sim_paused = false;
    }
}

MonteCarloSimulation::MonteCarloSimulation(int iter, int durat, double dt, Asset stock, bool show_inc) : iterations(iter), duration(durat), stock(stock), increment(dt), show(show_inc) {}

double MonteCarloSimulation::estimateOption(Option option)
{
    printf("Running simulation of option price...\n");
    /*
    Given an option properties, evaluate it's profitability knowing that prof=0 should be the bsOptionPrice
    */
    std::vector<double> final_stock_prices;

    // Collect data from weiner process

    for (int i = 0; i < iterations; i++)
    {
        PRINT_STEP(show, "iteration %d\n", i);
        WeinerProcessSimulator wps(stock.price, stock.drift, stock.volatility, increment, true);
        wps.runSimulation(duration, 0, show);
        final_stock_prices.push_back(wps.getPrice());
    }

    std::vector<double> option_profit;
    double profit;
    for (const double &value : final_stock_prices)
    {
        if (option.call)
        {
            profit = std::max(value - option.strike, 0.0);
        }
        else
        {
            profit = std::max(option.strike - value, 0.0);
        }
        option_profit.push_back(profit);
    }

    double average_profit = calculateAverage(option_profit);
    return average_profit;
}

void runMonteCarloThread(MonteCarloSimulation &mcs, Option &option)
{
    int i = 0;
    mcs_running = true;
    double discount_rate = exp(- mcs.stock.interest_rate * option.t);
    std::vector<double> options_profit;
    while (!mcs_stop && i < mcs.iterations)
    {
        ++i;

        {
            
            std::lock_guard<std::mutex> lock(mcs_mutex);
            WeinerProcessSimulator wps(mcs.stock.price, mcs.stock.drift, mcs.stock.volatility, mcs.increment, true);
            wps.runSimulation(mcs.duration, 0, false);
            double option_profit = option.call ? std::max(wps.getPrice() - option.strike, 0.0) : std::max(option.strike - wps.getPrice(), 0.0);
            options_profit.push_back(discount_rate*option_profit);


            if (i % 10 == 0)
            {
                //OPTIMISABLE!!!
                mcs_approx_price = calculateAverage(options_profit);
                mcs_progress = (double)i / mcs.iterations;
            }
        }
    }
    mcs_progress = 100.0;
    mcs_approx_price = calculateAverage(options_profit);
    mcs_running =false;
}

void stopMonteCarloThread(){
    if (mcs_running){
        mcs_stop =true;
        if (mcs_thread.joinable()){
            mcs_thread.join();

        }
        mcs_stop =false;
        mcs_running =false;
        
    }
}