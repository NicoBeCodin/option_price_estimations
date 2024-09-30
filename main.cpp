#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include "functions.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define WINDOW_WIDTH = 800;
#define WINDOW_HEIGHT = 400;


/*FUTURE IDEAS
More choice of distributions, not just simple weiner process
Weiner process itself is can be rewritten with fourier series

TODO:
change i++ to ++i

*/


bool InitOpenGL(GLFWwindow** window);
void Cleanup(GLFWwindow** window);


int main(){

    printf("Launching simulation... \n");

/*
    Option pricing simulation 
*/
    //Model the volatility of a stock on a month
    const std::string stock_name = "ABC";
    const double stock_initial_price = 100.0;
    const double stock_volatility = 0.6; //corresponds to an IV of 60%
    const double stock_drift = 0.0;
    Asset test_stock = {stock_name , stock_initial_price, stock_drift, stock_volatility};
    const bool show_inc =false;
    std::cout<< "Stock name : " << stock_name <<std::endl;
    printf("price %f , vol %f \n", stock_initial_price, stock_volatility);
    
    //Number of stocks simulated
    const int trials = 300;
    const int duration = 100; //Duration of the simulation (how many steps)
    const double real_time = 1.0;//how much time compared to a year is the stock simulated from
    const double step_size = real_time/(double)duration;
    printf("Number of stocks simulated: %d , duration of simulation: %d , time step %f\n", trials, duration, step_size);
    MonteCarloSimulation mcs(trials, duration, step_size, test_stock, show_inc);
    const double option_premium = 8.0;
    const double option_strike = 110.0;
    const double risk_free = 0.04;
    
    const double option_time_to_exp = real_time; //Time to exp is linked to duration of a trial
    const int step_number = 10; //number of steps for binomial pricing
    const bool show_pricing_steps= false;

    Option test_call = {test_stock, true, option_premium, option_strike, option_time_to_exp};
    
    double option_expected_premium = exp(-risk_free * real_time) * mcs.estimateOption(test_call);
    double bsPrice = bsOptionPrice(true, stock_initial_price, option_strike, risk_free, option_time_to_exp, stock_volatility);
    double americanOptionPrice = binomialOptionPrice(true, stock_initial_price, option_strike, risk_free, option_time_to_exp, stock_volatility, step_number, show_pricing_steps);
    
    printf("stock_init_price %f , option_premium %f, option_strike %f , r = %f, option_time_to_exp %f, stock_volatility %f \n",stock_initial_price, option_premium, option_strike, risk_free, option_time_to_exp, stock_volatility);
    printf("B-S price: %f\nMontecarlo price: %f\nAmerican option price w/ binomial pricing: %f\n", bsPrice, option_expected_premium, americanOptionPrice);
    

    
    //Graphics rendering

    //tries to init the glfw lib
    if (!glfwInit()){
        return -1;
    }

    //Tries to create a new window
    GLFWwindow* window =glfwCreateWindow(1280, 720, "ImGui Test", NULL, NULL);
    if (!window){
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();

    //Stock
    std::string sim_stock_name ="ABC";
    double stock_init_price = 100.0;
    double stock_vol = 0.6;
    double stock_dri = 0.0;
    
    double sim_option_strike = 110.0;
    bool call =true;
    
    //Simulation characteristics
    int n_trials = 100;
    int n_trial_steps = 200;
    double t_sim = 1.0;
    bool show = false;
    

    double interest_rate = 0.04;


    double result = 0.0;
    bool show_result =false;

    //GameLoop
    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("MonteCarlo Simulation");


        ImGui::InputDouble("Initial stock price: ", &stock_init_price);
        ImGui::InputDouble("Stock volatililty: ", &stock_vol);
        ImGui::InputDouble("Stock drift: ", &stock_dri);
        ImGui::InputDouble("Strike price: ", &sim_option_strike);
        ImGui::Checkbox("Call: (empty means put)", &call);

        ImGui::InputInt("Number of trials: ", &n_trials);
        ImGui::InputInt("Number of steps per trial: ", &n_trial_steps);
        ImGui::InputDouble("Time to expiration (years): ", &t_sim);
        ImGui::Checkbox("Show steps in simulation:", &show);

        if (ImGui::Button("Calculate")){
            Asset simulated_stock = {stock_name, stock_init_price,stock_dri, stock_vol};
            double step_size = t_sim / (double)n_trial_steps;
            MonteCarloSimulation mc_sim(n_trials, n_trial_steps, step_size, simulated_stock, show);
            Option sim_option;
            sim_option.stock = simulated_stock;
            sim_option.call =true;
            sim_option.strike = sim_option_strike;
            
            result = exp(-interest_rate* t_sim)* mc_sim.estimateOption(sim_option);
            show_result = true;
        }

        if (show_result){
            ImGui::Text("Result: %.3f", result);
            
        }
        ImGui::End();
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);


    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}