#include <iostream>
#include <GLFW/glfw3.h>
#include "functions.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"




/*FUTURE IDEAS
Model any noraml distribution with parameters for kurtosis

TODO:
change i++ to ++i
make clean function file
*/

/*
MonteCarloProcess
Run an experience n times and see what aoption price is best
*/


int main(){

    printf("Launching simulation... \n");

/*
    Option pricing simulation 
*/
    //Model the volatility of a stock on a month
    char* stock_name = "ABC";
    const double stock_initial_price = 100.0;
    const double stock_volatility = 0.6; //corresponds to an IV of 60%
    const double stock_drift = 0.0;
    Asset test_stock = {stock_name , stock_initial_price, stock_drift, stock_volatility};
    printf("Stock initial name %s,  price %f , vol %f \n", stock_name, stock_initial_price, stock_volatility);
    
    //Number of stocks simulated
    const int trials = 300;
    //Duration of the simulation (how many steps)
    const double duration = 1000;
    //how much time compared to a year is the stock simulated from
    const double real_time = 1.0;
    const double step_size = real_time/duration;

    printf("Number of stocks simulated: %d , duration of simulation: %f , time step %f\n", trials, duration, step_size );


    MonteCarloSimulation mcs(trials, duration, step_size, test_stock);
    const double option_premium = 8.0;
    const double option_strike = 110.0;
    const double risk_free = 0.04;
    //Time to exp is linked to duration of a trial
    const double option_time_to_exp = real_time;

    printf("Option premium : %f, strike %f , time to expriry: %f\n", option_premium, option_strike, option_time_to_exp);

    Option test_call = {test_stock, true, option_premium, option_strike, option_time_to_exp};
    double average_profit = mcs.simulateOption(test_call);

    double option_expected_premium  = option_premium + average_profit;

    double bsPrice = bsOptionPrice(true, stock_initial_price, option_strike, risk_free, option_time_to_exp, stock_volatility);
    double americanOptionPrice = binomialOptionPrice(true, stock_initial_price, option_strike, risk_free, option_time_to_exp, stock_volatility, 100);


    
    printf("stock_init_price %f , option_premium %f, option_strike %f , r = %f, option_time_to_exp %f, stock_volatility %f \n",stock_initial_price, option_premium, option_strike, risk_free, option_time_to_exp, stock_volatility);
    printf("B-S price: %f\nMontecarlo price: %f\n American option price w/ binomial pricing: %f\n", bsPrice, option_expected_premium, americanOptionPrice);
    return 0;
}