#pragma once

#include "WeightedGraph.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class ACOMode {
    ACOOnly,          // Construction + pheromone update, no local search.
    FixedTwoOpt,      // Local search on top-q tours using 2-opt only.
    FixedOrOpt,       // Local search on top-q tours using Or-opt only.
    FixedCombined,         // One 2-opt pass then one Or-opt pass on top-q tours.
    AdaptiveBandit,   // PDF's integrated top-q + fractional Beta bandit.
    SequentialBandit  // ACO first; bandit local search at the end.
};

const char* aco_mode_name(ACOMode mode);
ACOMode parse_aco_mode(const std::string& name);

struct ACOConfig {
    std::size_t ants = 30;                  // m
    std::size_t iterations = 200;            // T_ACO
    double pheromone_weight = 1.0;           // a in the PDF
    double heuristic_weight = 3.0;           // b in the PDF
    double evaporation_rate = 0.2;             // rho in the PDF
    double deposit_constant = 1.0;            // Q in the PDF
    std::size_t top_q = 2;                   // q
    double bandit_forgetting = 0.99;           // gamma in the PDF
    std::size_t no_improvement_window = 90;  // W local-search calls; 0 disables it
    double epsilon_length = 1e-12;            // epsilon_L in the PDF
    double epsilon_evaluations = 1e-12;       // epsilon_c in the PDF
    std::size_t diversification_interval = 50; // Stagnation-triggered partial restart; 0 disables it
    double diversification_fraction = 0.5;   // Blend toward a flat pheromone level
    std::uint32_t seed = 7;
    ACOMode mode = ACOMode::AdaptiveBandit;
};

struct TourResult {
    std::vector<std::size_t> tour; // Vertex indices; start is not repeated.
    double length = 0.0;           // Includes the closing edge back to the start.
    std::size_t iterations = 0;
    std::size_t local_search_calls = 0;
    std::size_t candidate_evaluations = 0;
    std::array<std::size_t, 3> arm_selections{0, 0, 0}; // 2-opt, Or-opt, combined
    std::size_t diversification_restarts = 0;
    bool stopped_for_no_improvement = false;
    double final_pheromone_min = 0.0;
    double final_pheromone_max = 0.0;
    double final_tau_min_bound = 0.0;
    double final_tau_max_bound = 0.0;
    std::array<double, 3> bandit_posterior_means{0.5, 0.5, 0.5};
    std::vector<std::size_t> arm_selection_history;
    std::vector<double> tour_length_stddev_by_iteration;
    std::vector<double> best_length_by_iteration;
    std::vector<std::size_t> local_search_calls_by_iteration;
    std::vector<std::size_t> candidate_evaluations_by_iteration;
};

// Computes the length of a cyclic TSP tour, including the closing edge.
double tour_length(const WeightedGraph& graph,
                   const std::vector<std::size_t>& tour);

// The PDF's reward transformation. Exposed to make its numerical guardrails testable.
double normalized_local_search_reward(double gain,
                                      double length_before,
                                      std::size_t candidate_evaluations,
                                      double epsilon_length = 1e-12,
                                      double epsilon_evaluations = 1.0);

// Solves a symmetric, complete TSP instance using the selected configuration.
TourResult aco_solve(const WeightedGraph& graph,
                     const ACOConfig& config = ACOConfig{});
