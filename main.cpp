#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>
#include <mutex>
#include "functions.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define WINDOW_WIDTH = 800;
#define WINDOW_HEIGHT = 400;

/*FUTURE IDEAS
More choice of distributions, not just simple weiner process
Weiner process itself is can be rewritten with fourier series
*/

bool InitOpenGL(GLFWwindow **window);
void Cleanup(GLFWwindow **window);

int main()
{

    printf("Launching program... \n");

    // Graphics rendering

    // tries to init the glfw lib
    if (!glfwInit())
    {
        return -1;
    }

    // Tries to create a new window
    GLFWwindow *window = glfwCreateWindow(1280, 720, "option pricing", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();

    // Stock
    std::string sim_stock_name = "ABC";
    double stock_init_price = 100.0;
    double stock_vol = 0.6;
    
    double black_scholes_price = 0.0;
    double american_option_price = 0.0;
    bool show_binomial = false;
    int tree_size = 100;
    double sim_option_strike = 110.0;
    bool call = true;
    
    bool show_mcs_result = false;

    double mcs_current_price;
    double mcs_progress_bar;
    int n_threads = std::thread::hardware_concurrency(); 
    double mcs_multithread_result =0.0;
    bool show_mcs_multithread_result =false;

    // Simulation characteristics
    int n_trials = 1000;
    int n_trial_steps = 500;
    double t_sim = 1.0;
    bool show = false;

    double sim_drift = 0.0;
    double sim_sigma = 0.5;
    double sim_step_size = 0.01;

    double interest_rate = 0.04;
    double stock_dri = interest_rate;

    bool show_bs_price = false;

    double init_price_process = 50.0;
    WeinerProcessSimulator wps(init_price_process, sim_drift, sim_sigma, sim_step_size, sim_stop);

    // GameLoop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Option estimator
        ImGui::SetNextWindowSize(ImVec2(200, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Option price estimator");

        ImGui::InputDouble("Initial stock price", &stock_init_price);
        ImGui::InputDouble("Stock volatililty", &stock_vol);
        ImGui::InputDouble("Stock drift (default is r)", &stock_dri);
        ImGui::InputDouble("Strike price ", &sim_option_strike);
        ImGui::InputDouble("risk free interest rate", &interest_rate);
        ImGui::InputDouble("Time to expiration (years)", &t_sim);
        ImGui::Checkbox("Call: (empty means put)", &call);

        ImGui::InputInt("Number of trials", &n_trials);
        ImGui::InputInt("Number of steps per trial", &n_trial_steps);
        ImGui::InputInt("thread number ", &n_threads);
    
        ImGui::Checkbox("Show steps in simulation", &show);
        ImGui::InputInt("Binomial tree size", &tree_size);


        if (ImGui::Button("Run multithreaded MonteCarlo simulation")){
            stopMonteCarloMultiThread();
            Asset simulated_stock = {"ABC", stock_init_price, stock_dri, stock_vol, interest_rate};

            double step_size = t_sim / (double)n_trial_steps;
            
            MonteCarloSimulation mc_sim_multithread(n_trials, n_trial_steps, step_size, simulated_stock, show);
            Option sim_option;
            sim_option.stock = simulated_stock;
            sim_option.call = call;
            sim_option.strike = sim_option_strike;
            

            mcs_multithread = std::thread([&]() {
        // This runs the simulation on a separate thread and stores the result
            
            mcs_multithread_result = exp(-interest_rate * t_sim) * runMonteCarloMultiThreading(n_threads, std::ref(mc_sim_multithread), sim_option);
        
        // Once the simulation is finished, update the running flag

            });
            
            show_mcs_multithread_result =true;

        }

        if (ImGui::Button("Calculate Black-Scholes Price"))
        {
            show_bs_price = true;
            black_scholes_price = bsOptionPrice(call, stock_init_price, sim_option_strike, interest_rate, t_sim, stock_vol);
        }

        if (ImGui::Button("Calculate binomial tree"))
        {
            show_binomial = true;
            american_option_price = binomialOptionPrice(call, stock_init_price, sim_option_strike, interest_rate, t_sim, stock_vol, tree_size = 100, false);
        }

        if (show_mcs_result)
        {

            if (mcs_running && !mcs_finish)
            {
                {
                    std::lock_guard<std::mutex> lock(mcs_mutex);
                    mcs_current_price = mcs_approx_price;
                    mcs_progress_bar = mcs_progress;
                }

                ImGui::Text("Simulation progress %.1f % \n Approximating price: %.2f ", mcs_progress_bar * 100.0, mcs_current_price);
            }
            else if (mcs_finish)
            {
                ImGui::Text("MonteCarlo simulation price estimate: %.2f", mcs_current_price);
            }
        }
        if (show_mcs_multithread_result){
            if (mcs_multithread_running){
                float progress = (float)mcs_multithread_progress / (float)n_trials;
                ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
             } else {
                
                ImGui::Text("Multithread MonteCarlo simulation price estimate: %.2f", mcs_multithread_result);
             }
        }

        if (show_bs_price)
        {
            ImGui::Text("Black-Scholes price: %.2f", black_scholes_price);
        }

        if (show_binomial)
        {
            ImGui::Text("binomial tree for american option price: %.2f", american_option_price);
        }

        ImGui::End();
        // End of first window

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