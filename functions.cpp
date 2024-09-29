
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <cmath>
#include <vector>
#include <algorithm>
#include "functions.h"

double normalCDF(double x) {
    return 0.5 * erfc(-x * sqrt(2));
}

double bsOptionPrice(bool call, double s, double k, double r, double t, double sigma){
    double d1 = (log(s / k) + (r + 0.5 * sigma * sigma) * t) / (sigma * sqrt(t));
    double d2 = d1 - sigma * sqrt(t);

    //printf("d1: %f d2: %f \n", d1, d2);

    if (call){
        return (normalCDF(d1)*s - normalCDF(d2)*k*(exp(-r * t)));
    }
    else {
        return (normalCDF(-d2) * k * exp(-r*t) - normalCDF(-d1)*s);
    }
}


double binomialOptionPrice(bool call, double s, double k, double r, double t, double sigma, int n){
    double time_step = t / (double)n;
    double up_factor = exp(sigma * sqrt(time_step));
    double down_factor = 1.0 / up_factor;
    double risk_neutral_prob = (exp(r * time_step) - down_factor)/(up_factor - down_factor);
    double discount_factor = exp(- r* time_step);

    //std::cout << "Up_ factor: " << up_factor << " Down_factor: " << down_factor<<" risk_neutral_prob: "<< risk_neutral_prob <<std::endl;
    

    //Init 2D array for stock price and option price
    std::vector<std::vector<double>> stock_prices(n+1, std::vector<double>(n+1));
    std::vector<std::vector<double>> option_values(n+1, std::vector<double>(n+1));
    //All possible stocks for n length of simulation (binomial tree, nth step has n+1 possible outputs (See pascal triangle))
    for (int i=0; i<=n; ++i){
        for (int j= 0; j<=i;++j){
            stock_prices[i][j] = s * pow(up_factor, j)* pow(down_factor, i-j); //S0 * u^j * d^(i-j)
            //std::cout << "i: "<<i<<" j: "<<j<<" price: "<< stock_prices[i][j]<<std::endl;
            
        }
    }

    //Initialize option value from expiration
    for (int j =0; j<=n; ++j){
        if (call){
            
            option_values[n][j] = std::max(stock_prices[n][j] - k, 0.0);
            //std::cout << "call option j: " << j << " option_values: " << option_values[n][j] <<std::endl;
        }
        else {
            option_values[n][j] = std::max(k - stock_prices[n][j], 0.0);
        }

    }
    //Work backwards 
    //std::cout<<" Backwards "<< std::endl;
    
    for (int i = n-1; i>=0; --i){
        for (int j=i; j>=0; --j){
            double hold_value = discount_factor * (risk_neutral_prob  * option_values[i+1][j+1] + (1.0 - risk_neutral_prob)* option_values[i+1][j]);
            double exercise_value = 0.0;
            //std::cout << "i: " << i << " j: " << j << " hold_value " << hold_value << std::endl;

            if (call){
                exercise_value = std::max(stock_prices[i][j] - k, 0.0);
                //std::cout << "i: " << i << " j: " << j << " exercise_value " << exercise_value << std::endl;
                

            } else {
                exercise_value = std::max(k - stock_prices[i][j], 0.0);

            }

            option_values[i][j] = std::max(hold_value, exercise_value);

            // if (hold_value<exercise_value){
            //     printf("EX > HOLD !!!\n");
            // }
            
            //std::cout << "i: " << i << " j: " << j << " stock_values " << stock_prices[i][j] << std::endl;
            //std::cout << "i: " << i << " j: " << j << " option_values " << option_values[i][j] << std::endl;
            printf("\n");
        }
    }

    return option_values[0][0];
}

double calculateAverage(const std::vector<double>& values){
    if (values.empty()){
        throw std::invalid_argument("Empty list, can't calculate average");
    }
    double sum =0.0;
    for (const double &val : values){
        sum+=val;
    }
    return sum / values.size();
}


//refine this func
double impliedVolatility(double S, double K, double T, double r, double marketPrice, double tol = 1e-6, int maxIter = 1000) {
    double lowVol = 0.0;
    double highVol = 5.0;  // Start with a high guess for volatility
    double midVol = 0.0;

    for (int i = 0; i < maxIter; ++i) {
        midVol = (lowVol + highVol) / 2.0;
        double price = bsOptionPrice(true, S, K, T, r, midVol);

        if (fabs(price - marketPrice) < tol) {
            return midVol;  // Solution found
        }

        if (price > marketPrice) {
            highVol = midVol;  // Volatility too high, reduce it
        } else {
            lowVol = midVol;  // Volatility too low, increase it
        }
    }

    // If it doesn't converge, return the best guess
    return midVol;
}
WeinerProcessSimulator::WeinerProcessSimulator(double initialPrice,double drift, double volatility, double timeStep): price(initialPrice), mu(drift), sigma(volatility), dt(timeStep) {}

void WeinerProcessSimulator::simulateStep(bool show){
        double randomValue = generateNormal(0.0, 1.0);
        const double dW = randomValue * sqrt(dt);
        const double old_price = price;
        price = price * exp((mu - 0.5*sigma*sigma)*dt + sigma *dW);
        double increment = price - old_price;
        if (show){
            std::cout<< "Price : "<< price << " Increment : " << increment << std::endl;
        }
    }

void WeinerProcessSimulator::runSimulation(int n, int delay_ms, bool show){
        for (int i=0; i<n; i++){
            simulateStep(show);
            if (delay_ms != 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            }          
        }
    }


MonteCarloSimulation::MonteCarloSimulation(int iter, double durat, double dt, Asset stock): iterations(iter), duration(durat), increment(dt), stock(stock){}

double MonteCarloSimulation::simulateOption(Option option){
    printf("Running simulation of option price...\n");
        /*
        Given an option properties, evaluate it's profitability knowing that prof=0 should be the bsOptionPrice
        */
        std::vector<double> final_stock_prices;

        //Collect data from weiner process
        for (int i=0; i<iterations; i++){

            WeinerProcessSimulator wps(stock.price, stock.drift, stock.volatility, increment);
            wps.runSimulation(duration, 0, false);
            final_stock_prices.push_back(wps.getPrice());

        }

        std::vector<double> option_profit;
        double profit;
        for (const double &value: final_stock_prices){
            if (option.call){
                profit = std::max(value - option.strike, 0.0);
            } else {
                profit = std::max(option.strike - value, 0.0);
            }
            option_profit.push_back(profit);
        }

        double average_profit = calculateAverage(option_profit);
        double expected_value = average_profit - option.premium;
        return expected_value;
}

double WeinerProcessSimulator::generateNormal(double mean, double stddev){
        std::normal_distribution<> d(mean, stddev);
        return d(gen);
    }


double generateNormal(double mean, double stddev){
    static std::random_device rd;
    std::mt19937 gen(rd()); //Mersenne twister
    std::normal_distribution<> d(mean, stddev);
    return d(gen);
}

double weiner_process(double price, double dt, double sigma){
    double random = generateNormal(0.0, 1.0);
    return sigma * sqrt(dt)* random;

}

