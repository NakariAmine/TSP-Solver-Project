// ACO algorithm with adaptive local search.
//
// Overview:
// 1. Each ant constructs a tour.
// 2. Sort tours from shortest to longest.
// 3. Depending on the selected mode, improve some tours using 2-opt, Or-opt, or combined 2-opt + Or-opt.
// 4. Ants deposit pheromone on their edges.
// 5. Repeat until reaching the iteration limit or the stopping condition.

#include "ACO_Solver.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kDistanceEpsilon = 1e-12;

enum class LocalSearchArm : std::size_t {
    TwoOpt = 0,
    OrOpt = 1,
    Combined = 2
};

// An ant is represented by its tour and the length of that tour.
struct Ant {
    std::vector<std::size_t> tour;
    // Initially, the length is infinity: the tour has not been evaluated yet.
    double length = std::numeric_limits<double>::infinity();
};

// The bandit stores two parameters per operator, defining its Beta distribution.
struct BetaArm {
    double alpha = 1.0;
    double beta = 1.0;
};

// Measurements collected during one local-search call.
struct LocalSearchResult {
    double before = 0.0;       // Length before local search.
    double after = 0.0;        // Length after local search.
    std::size_t evaluations = 0; // Number of candidate moves evaluated.
};

// Sort ants from the shortest tour to the longest.
// Insertion sort: place each ant in the correct position.
// The number of ants is usually small (30 by default), so this sort is sufficient.
void sort_ants_by_length(std::vector<Ant>& ants)
{
    for (std::size_t i = 1; i < ants.size(); ++i) {
        Ant current_ant = ants[i];
        std::size_t position = i;

        while (position > 0 &&
               ants[position - 1].length > current_ant.length) {
            ants[position] = ants[position - 1];
            --position;
        }

        ants[position] = current_ant;
    }
}

// Return true if every call in the window produced no improvement.
bool no_recent_improvement(const std::deque<bool>& recent_gains)
{
    for (bool has_improved : recent_gains) {
        if (has_improved) {
            return false;
        }
    }
    return true;
}

// Return the distance between two cities.
double distance_between(const WeightedGraph& graph,
                        std::size_t from,
                        std::size_t to)
{
    return graph.weight_between_vertices(from, to);
}

std::vector<std::size_t> construct_tour(
    const WeightedGraph& graph,
    const std::vector<std::vector<double>>& pheromone,
    double pheromone_weight,
    double heuristic_weight,
    std::mt19937& rng)
{
    const std::size_t n = graph.size();
    std::vector<std::size_t> tour;
    tour.reserve(n);
    if (n == 0) {
        return tour;
    }

    std::uniform_int_distribution<std::size_t> choose_start(0, n - 1);
    const std::size_t start = choose_start(rng);
    tour.push_back(start);

    std::vector<bool> visited(n, false);
    visited[start] = true;

    while (tour.size() < n) {
        const std::size_t current = tour.back();
        std::vector<std::size_t> candidates;
        std::vector<double> weights;
        candidates.reserve(n - tour.size());
        weights.reserve(n - tour.size());
        double total_weight = 0.0;

        for (std::size_t next = 0; next < n; ++next) {
            if (visited[next]) {
                continue;
            }

            const double d = distance_between(graph, current, next);
            const double safe_distance = std::max(d, kDistanceEpsilon);
            const double tau = std::max(pheromone[current][next], kDistanceEpsilon);
            double weight = std::pow(tau, pheromone_weight)
                          * std::pow(1.0 / safe_distance, heuristic_weight);
            if (!std::isfinite(weight) || weight < 0.0) {
                weight = 0.0;
            }

            candidates.push_back(next);
            weights.push_back(weight);
            total_weight += weight;
        }

        if (candidates.empty()) {
            break;
        }

        std::size_t chosen_position = 0;
        if (!(total_weight > 0.0) || !std::isfinite(total_weight)) {
            std::uniform_int_distribution<std::size_t> choose_candidate(
                0, candidates.size() - 1);
            chosen_position = choose_candidate(rng);
        } else {
            std::discrete_distribution<std::size_t> distribution(
                weights.begin(), weights.end());
            chosen_position = distribution(rng);
        }

        const std::size_t chosen = candidates[chosen_position];
        tour.push_back(chosen);
        visited[chosen] = true;
    }

    return tour;
}

// 2-opt: replace two edges and reverse the tour segment between them.
// Scan once without restarting; accept a move only if it shortens the tour.
std::size_t two_opt(std::vector<std::size_t>& tour,
                    const WeightedGraph& graph)
{
    const std::size_t n = tour.size();
    std::size_t evaluations = 0;
    if (n < 4) {
        return evaluations;
    }

    for (std::size_t i = 0; i + 2 < n; ++i) {
        for (std::size_t j = i + 2; j < n; ++j) {
            if (i == 0 && j == n - 1) {
                continue; // These two edges are already adjacent in the cycle.
            }

            // Refresh the edges after any earlier reversal in this pass.
            const std::size_t a = tour[i];
            const std::size_t b = tour[(i + 1) % n];
            const std::size_t c = tour[j];
            const std::size_t d = tour[(j + 1) % n];
            ++evaluations;

            const double old_edges = distance_between(graph, a, b)
                                   + distance_between(graph, c, d);
            const double new_edges = distance_between(graph, a, c)
                                   + distance_between(graph, b, d);
            if (new_edges < old_edges - kDistanceEpsilon) {
                std::reverse(tour.begin() + static_cast<std::ptrdiff_t>(i + 1),
                             tour.begin() + static_cast<std::ptrdiff_t>(j + 1));
            }
        }
    }
    return evaluations;
}

// Or-opt: relocate a segment of 1, 2, or 3 cities to another position.
// Scan once without restarting; keep a move only if it improves the tour.
std::size_t or_opt(std::vector<std::size_t>& tour,
                   const WeightedGraph& graph)
{
    const std::size_t n = tour.size();
    std::size_t evaluations = 0;
    if (n < 4) {
        return evaluations;
    }

    for (std::size_t segment_size = 1;
         segment_size <= 3 && segment_size + 1 < n;
         ++segment_size) {
        for (std::size_t i = 0; i + segment_size <= n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                // Do not reinsert the segment into itself or at its current position.
                if ((j >= i && j < i + segment_size) ||
                    j == (i + n - 1) % n) {
                    continue;
                }

                // Earlier relocations change the segment at position i.
                const std::size_t previous = tour[(i + n - 1) % n];
                const std::size_t first = tour[i];
                const std::size_t last = tour[i + segment_size - 1];
                const std::size_t next = tour[(i + segment_size) % n];

                const std::size_t anchor = tour[j];
                const std::size_t anchor_next = tour[(j + 1) % n];
                ++evaluations;

                const double removed = distance_between(graph, previous, first)
                                     + distance_between(graph, last, next)
                                     + distance_between(graph, anchor, anchor_next);
                const double added = distance_between(graph, previous, next)
                                   + distance_between(graph, anchor, first)
                                   + distance_between(graph, last, anchor_next);

                if (added < removed - kDistanceEpsilon) {
                    std::vector<std::size_t> segment(
                        tour.begin() + static_cast<std::ptrdiff_t>(i),
                        tour.begin() + static_cast<std::ptrdiff_t>(i + segment_size));
                    std::vector<std::size_t> candidate = tour;
                    candidate.erase(
                        candidate.begin() + static_cast<std::ptrdiff_t>(i),
                        candidate.begin() + static_cast<std::ptrdiff_t>(i + segment_size));
                    std::vector<std::size_t>::iterator anchor_it =
                        std::find(candidate.begin(), candidate.end(), anchor);
                    if (anchor_it == candidate.end()) {
                        throw std::logic_error("Or-opt anchor disappeared from tour");
                    }
                    std::vector<std::size_t>::iterator insert_at = anchor_it;
                    ++insert_at; // Insert immediately after the anchor city.
                    candidate.insert(insert_at, segment.begin(), segment.end());
                    // Final check: accept only if the actual tour length
                    // decreases. Or-opt therefore cannot make the tour worse.
                    if (tour_length(graph, candidate) < tour_length(graph, tour) - kDistanceEpsilon) {
                        tour = candidate;
                    }
                }
            }
        }
    }
    return evaluations;
}

LocalSearchResult apply_local_search(LocalSearchArm arm,
                                    std::vector<std::size_t>& tour,
                                    const WeightedGraph& graph)
{
    LocalSearchResult result;
    result.before = tour_length(graph, tour);

    if (arm == LocalSearchArm::TwoOpt) {
        result.evaluations = two_opt(tour, graph);
    } else if (arm == LocalSearchArm::OrOpt) {
        result.evaluations = or_opt(tour, graph);
    } else {
        result.evaluations = two_opt(tour, graph);
        result.evaluations += or_opt(tour, graph);
    }

    result.after = tour_length(graph, tour);
    return result;
}

// Draw a random value from a Beta(alpha, beta) distribution.
// The C++ standard library has no direct Beta generator: use two
// independent Gamma draws, then compute G1 / (G1 + G2).
double sample_beta(const BetaArm& arm, std::mt19937& rng)
{
    std::gamma_distribution<double> gamma_alpha(arm.alpha, 1.0);
    std::gamma_distribution<double> gamma_beta(arm.beta, 1.0);
    const double x = gamma_alpha(rng);
    const double y = gamma_beta(rng);
    const double sum = x + y;
    if (!(sum > 0.0) || !std::isfinite(sum)) {
        return arm.alpha / (arm.alpha + arm.beta);
    }
    return x / sum;
}

// Thompson sampling: draw one value per operator and choose
// the operator with the highest draw.
LocalSearchArm select_arm(const std::array<BetaArm, 3>& arms,
                          std::mt19937& rng)
{
    std::size_t best_index = 0;
    double best_sample = -1.0;
    for (std::size_t i = 0; i < arms.size(); ++i) {
        const double sample = sample_beta(arms[i], rng);
        if (sample > best_sample) {
            best_sample = sample;
            best_index = i;
        }
    }
    return static_cast<LocalSearchArm>(best_index);
}

void update_bandit(BetaArm& arm, double reward, double forgetting)
{
    arm.alpha = forgetting * arm.alpha + reward;
    arm.beta = forgetting * arm.beta + (1.0 - reward);
    arm.alpha = std::max(arm.alpha, kDistanceEpsilon);
    arm.beta = std::max(arm.beta, kDistanceEpsilon);
}

// Keep each pheromone value between tau_min and tau_max (MMAS bounds).
void clamp_pheromone(std::vector<std::vector<double>>& pheromone,
                     double tau_min,
                     double tau_max)
{
    for (std::size_t i = 0; i < pheromone.size(); ++i) {
        pheromone[i][i] = 0.0;

        for (std::size_t j = i + 1; j < pheromone.size(); ++j) {
            double value = pheromone[i][j];

            if (value < tau_min) {
                value = tau_min;
            }
            if (value > tau_max) {
                value = tau_max;
            }

            pheromone[i][j] = value;
            pheromone[j][i] = value;
        }
    }
}

std::vector<std::size_t> canonical_tour(const std::vector<std::size_t>& tour)
{
    if (tour.empty()) {
        return {};
    }
    std::vector<std::size_t>::const_iterator minimum_it =
        std::min_element(tour.begin(), tour.end());
    const std::size_t start = static_cast<std::size_t>(minimum_it - tour.begin());
    std::vector<std::size_t> forward;
    std::vector<std::size_t> reverse;
    forward.reserve(tour.size());
    reverse.reserve(tour.size());
    for (std::size_t offset = 0; offset < tour.size(); ++offset) {
        forward.push_back(tour[(start + offset) % tour.size()]);
        reverse.push_back(tour[(start + tour.size() - offset) % tour.size()]);
    }
    return std::min(forward, reverse);
}

// For the sequential method, keep an archive of the best distinct tours.
// The map makes it easy to recognize a previously seen tour.
void update_archive(std::map<std::vector<std::size_t>, Ant>& archive,
                    const Ant& candidate,
                    std::size_t capacity)
{
    const std::vector<std::size_t> key = canonical_tour(candidate.tour);
    std::map<std::vector<std::size_t>, Ant>::iterator found = archive.find(key);
    if (found != archive.end()) {
        if (candidate.length < found->second.length) {
            found->second = candidate;
        }
    } else {
        archive[key] = candidate;
    }

    if (archive.size() > capacity) {
        std::map<std::vector<std::size_t>, Ant>::iterator worst = archive.begin();
        std::map<std::vector<std::size_t>, Ant>::iterator current = archive.begin();
        ++current;

        while (current != archive.end()) {
            if (current->second.length > worst->second.length) {
                worst = current;
            }
            ++current;
        }
        archive.erase(worst);
    }
}

// Update bandit statistics after a local-search call.
void observe_local_search(const LocalSearchResult& local,
                          std::array<BetaArm, 3>& arms,
                          LocalSearchArm arm,
                          const ACOConfig& config,
                          TourResult& result,
                          std::deque<bool>& recent_gains)
{
    const double gain = std::max(0.0, local.before - local.after);
    const double reward = normalized_local_search_reward(
        gain, local.before, local.evaluations,
        config.epsilon_length, config.epsilon_evaluations);

    const std::size_t arm_index = static_cast<std::size_t>(arm);
    update_bandit(arms[arm_index], reward, config.bandit_forgetting);
    ++result.local_search_calls;
    ++result.arm_selections[arm_index];
    result.arm_selection_history.push_back(arm_index);
    result.candidate_evaluations += local.evaluations;

    if (config.no_improvement_window > 0) {
        recent_gains.push_back(gain > 0.0);
        while (recent_gains.size() > config.no_improvement_window) {
            recent_gains.pop_front();
        }
    }
}

void consider_best(const Ant& ant, TourResult& best)
{
    if (best.tour.empty() || ant.length < best.length) {
        best.tour = ant.tour;
        best.length = ant.length;
    }
}

// If the search stagnates, blend pheromone values toward a common level.
void partially_reinitialize_pheromone(std::vector<std::vector<double>>& pheromone,
                                      double tau_min,
                                      double tau_max,
                                      double fraction)
{
    // Move pheromone values toward a middle level without erasing all information.
    const double target = 0.5 * (tau_min + tau_max);
    for (std::size_t i = 0; i < pheromone.size(); ++i) {
        pheromone[i][i] = 0.0;
        for (std::size_t j = i + 1; j < pheromone.size(); ++j) {
            const double updated = (1.0 - fraction) * pheromone[i][j]
                                 + fraction * target;
            pheromone[i][j] = updated;
            pheromone[j][i] = updated;
        }
    }
    clamp_pheromone(pheromone, tau_min, tau_max);
}

} // namespace

const char* aco_mode_name(ACOMode mode)
{
    switch (mode) {
    case ACOMode::ACOOnly: return "aco-only";
    case ACOMode::FixedTwoOpt: return "aco-fixed-2opt";
    case ACOMode::FixedOrOpt: return "aco-fixed-oropt";
    case ACOMode::FixedCombined: return "aco-fixed-combined";
    case ACOMode::AdaptiveBandit: return "aco-bandit";
    case ACOMode::SequentialBandit: return "aco-sequential";
    }
    return "unknown";
}

ACOMode parse_aco_mode(const std::string& name)
{
    if (name == "aco-only") return ACOMode::ACOOnly;
    if (name == "aco-fixed" || name == "aco-fixed-2opt" || name == "aco-2opt") return ACOMode::FixedTwoOpt;
    if (name == "aco-fixed-oropt" || name == "aco-oropt") return ACOMode::FixedOrOpt;
    if (name == "aco-fixed-combined" || name == "aco-combined") return ACOMode::FixedCombined;
    if (name == "aco-bandit" || name == "aco") return ACOMode::AdaptiveBandit;
    if (name == "aco-sequential") return ACOMode::SequentialBandit;
    throw std::invalid_argument("Unknown ACO mode: " + name);
}

double tour_length(const WeightedGraph& graph,
                   const std::vector<std::size_t>& tour)
{
    if (tour.size() < 2) {
        return 0.0;
    }
    double length = 0.0;
    for (std::size_t i = 0; i < tour.size(); ++i) {
        length += distance_between(graph, tour[i], tour[(i + 1) % tour.size()]);
    }
    return length;
}

double normalized_local_search_reward(double gain,
                                      double length_before,
                                      std::size_t candidate_evaluations,
                                      double epsilon_length,
                                      double epsilon_evaluations)
{
    if (!(epsilon_length > 0.0) || !std::isfinite(epsilon_length) ||
        !(epsilon_evaluations > 0.0) || !std::isfinite(epsilon_evaluations)) {
        throw std::invalid_argument("Reward epsilons must be finite and strictly positive");
    }
    if (!std::isfinite(gain) || !std::isfinite(length_before)) {
        return 0.0;
    }

    const double nonnegative_gain = std::max(0.0, gain);
    const double relative_gain = nonnegative_gain / std::max(length_before, epsilon_length);
    const double cost = std::max(static_cast<double>(candidate_evaluations),
                                 epsilon_evaluations);
    const double raw_reward = relative_gain / cost;
    if (!(raw_reward > 0.0) || !std::isfinite(raw_reward)) {
        return 0.0;
    }
    return raw_reward / (1.0 + raw_reward);
}

TourResult aco_solve(const WeightedGraph& graph, const ACOConfig& config)
{
    const std::size_t n = graph.size();
    if (n == 0) throw std::invalid_argument("Cannot solve a TSP instance with no cities");
    if (config.ants == 0) throw std::invalid_argument("ACO needs at least one ant");
    if (config.iterations == 0) throw std::invalid_argument("ACO needs at least one iteration");
    if (!(config.evaporation_rate > 0.0 && config.evaporation_rate <= 1.0)) {
        throw std::invalid_argument("Evaporation rate must be in (0, 1]");
    }
    if (!(config.bandit_forgetting > 0.0 && config.bandit_forgetting < 1.0)) {
        throw std::invalid_argument("Bandit forgetting factor must be in (0, 1)");
    }
    if (config.top_q == 0) throw std::invalid_argument("top_q must be at least one");
    if (!std::isfinite(config.pheromone_weight) || !std::isfinite(config.heuristic_weight) ||
        config.pheromone_weight < 0.0 || config.heuristic_weight < 0.0) {
        throw std::invalid_argument("ACO exponents must be non-negative");
    }
    if (!(config.deposit_constant > 0.0) || !std::isfinite(config.deposit_constant)) {
        throw std::invalid_argument("Deposit constant Q must be finite and positive");
    }
    if (!(config.epsilon_length > 0.0) || !std::isfinite(config.epsilon_length) ||
        !(config.epsilon_evaluations > 0.0) || !std::isfinite(config.epsilon_evaluations)) {
        throw std::invalid_argument("Reward epsilons must be strictly positive");
    }
    if (!(config.diversification_fraction > 0.0 && config.diversification_fraction <= 1.0)) {
        throw std::invalid_argument("Diversification fraction must be in (0, 1]");
    }

    std::mt19937 rng(config.seed);
    const double initial_tau = 1.0;
    std::vector<std::vector<double>> pheromone(
        n, std::vector<double>(n, initial_tau));
    for (std::size_t i = 0; i < n; ++i) pheromone[i][i] = 0.0;

    // All three operators start with a Beta(1, 1) distribution.
    std::array<BetaArm, 3> arms{};
    std::deque<bool> recent_gains;
    const std::size_t top_q = std::min(config.top_q, config.ants);
    TourResult result;
    result.length = std::numeric_limits<double>::infinity();
    std::map<std::vector<std::size_t>, Ant> sequential_archive;
    const std::size_t max_size = std::numeric_limits<std::size_t>::max();
    const std::size_t sequential_archive_capacity =
        (top_q > 0 && config.iterations > max_size / top_q)
            ? max_size : top_q * config.iterations;
    std::size_t iterations_since_best = 0;
    double tau_min = 0.0;
    double tau_max = std::numeric_limits<double>::infinity();

    // Main ACO loop: one pass corresponds to one iteration.
    for (std::size_t iteration = 0; iteration < config.iterations; ++iteration) {
        const std::size_t calls_before_iteration = result.local_search_calls;
        const std::size_t evaluations_before_iteration = result.candidate_evaluations;
        std::vector<Ant> ants;
        ants.reserve(config.ants);

        // Step 1: each ant constructs a complete tour.
        for (std::size_t ant_index = 0; ant_index < config.ants; ++ant_index) {
            Ant ant;
            ant.tour = construct_tour(graph, pheromone,
                                      config.pheromone_weight,
                                      config.heuristic_weight, rng);
            ant.length = tour_length(graph, ant.tour);
            ants.push_back(ant);
        }
        // Place the best tours at the beginning of the array.
        sort_ants_by_length(ants);

        // In the sequential method, save the raw tours for improvement at the end.
        if (config.mode == ACOMode::SequentialBandit) {
            for (const Ant& ant : ants) update_archive(sequential_archive, ant, sequential_archive_capacity);
        }

        bool new_global_best = false;
        // Step 2: in the integrated version, improve the top-q tours
        // before pheromone deposition. Other ants keep their raw tours.
        if (config.mode == ACOMode::FixedTwoOpt || config.mode == ACOMode::FixedOrOpt ||
            config.mode == ACOMode::FixedCombined || config.mode == ACOMode::AdaptiveBandit) {
            for (std::size_t rank = 0; rank < top_q; ++rank) {
                const double previous_best_length = result.length;
                LocalSearchArm selected = LocalSearchArm::TwoOpt;
                if (config.mode == ACOMode::FixedOrOpt) {
                    selected = LocalSearchArm::OrOpt;
                } else if (config.mode == ACOMode::FixedCombined) {
                    selected = LocalSearchArm::Combined;
                } else if (config.mode == ACOMode::AdaptiveBandit) {
                    selected = select_arm(arms, rng);
                }
                LocalSearchResult local = apply_local_search(selected, ants[rank].tour, graph);
                ants[rank].length = local.after;

                if (config.mode == ACOMode::AdaptiveBandit) {
                    observe_local_search(local, arms, selected, config, result, recent_gains);
                } else {
                    ++result.local_search_calls;
                    const std::size_t selected_index = static_cast<std::size_t>(selected);
                    ++result.arm_selections[selected_index];
                    result.candidate_evaluations += local.evaluations;
                    const double gain = std::max(0.0, local.before - local.after);
                    if (config.no_improvement_window > 0) {
                        recent_gains.push_back(gain > 0.0);
                        while (recent_gains.size() > config.no_improvement_window) {
                            recent_gains.pop_front();
                        }
                    }
                }
                consider_best(ants[rank], result);
                if (result.length < previous_best_length) new_global_best = true;
            }
        }

        for (const Ant& ant : ants) {
            const double previous_best_length = result.length;
            consider_best(ant, result);
            if (result.length < previous_best_length) new_global_best = true;
        }
        if (new_global_best) iterations_since_best = 0;
        else ++iterations_since_best;

        // Diagnostic: standard deviation of all ant tour lengths in this iteration.
        double mean_length = 0.0;
        for (const Ant& ant : ants) mean_length += ant.length;
        mean_length /= static_cast<double>(ants.size());
        double variance_length = 0.0;
        for (const Ant& ant : ants) {
            const double difference = ant.length - mean_length;
            variance_length += difference * difference;
        }
        variance_length /= static_cast<double>(ants.size());
        result.tour_length_stddev_by_iteration.push_back(std::sqrt(variance_length));
        result.best_length_by_iteration.push_back(result.length);
        result.local_search_calls_by_iteration.push_back(
            result.local_search_calls - calls_before_iteration);
        result.candidate_evaluations_by_iteration.push_back(
            result.candidate_evaluations - evaluations_before_iteration);

        // Step 3: evaporation. Existing trails lose some of their strength.
        // tau_ij <- (1-rho) * tau_ij.
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i + 1; j < n; ++j) {
                pheromone[i][j] *= (1.0 - config.evaporation_rate);
                pheromone[j][i] = pheromone[i][j];
            }
        }

        // Step 4: deposition. A good (short) tour deposits more pheromone.
        // In integrated mode, the top-q tours have already been improved.
        for (const Ant& ant : ants) {
            if (!(ant.length > 0.0) || !std::isfinite(ant.length)) continue;
            const double deposit = config.deposit_constant / ant.length;
            for (std::size_t i = 0; i < ant.tour.size(); ++i) {
                const std::size_t from = ant.tour[i];
                const std::size_t to = ant.tour[(i + 1) % ant.tour.size()];
                pheromone[from][to] += deposit;
                if (from != to) pheromone[to][from] += deposit;
            }
        }

        if (std::isfinite(result.length) && result.length > 0.0) {
            tau_max = 1.0 / (config.evaporation_rate * result.length);
            tau_min = tau_max / (2.0 * static_cast<double>(n));
            clamp_pheromone(pheromone, tau_min, tau_max);
        }

        // Step 5: optional partial restart if the best solution stagnates.
        if (config.diversification_interval > 0 &&
            iterations_since_best > 0 &&
            iterations_since_best % config.diversification_interval == 0 &&
            std::isfinite(tau_max) && tau_max >= tau_min) {
            partially_reinitialize_pheromone(pheromone, tau_min, tau_max,
                                             config.diversification_fraction);
            ++result.diversification_restarts;
            iterations_since_best = 0;
        }

        result.iterations = iteration + 1;

        if ((config.mode == ACOMode::FixedTwoOpt || config.mode == ACOMode::FixedOrOpt ||
             config.mode == ACOMode::FixedCombined || config.mode == ACOMode::AdaptiveBandit) &&
            config.no_improvement_window > 0 &&
            recent_gains.size() == config.no_improvement_window &&
            no_recent_improvement(recent_gains)) {
            result.stopped_for_no_improvement = true;
            break;
        }
    }

    // Sequential method: local search starts only after the ACO loop.
    // Then apply bandit-selected local search to the best distinct archived tours.
    if (config.mode == ACOMode::SequentialBandit && !sequential_archive.empty()) {
        std::vector<Ant> archived_tours;
        archived_tours.reserve(sequential_archive.size());
        std::map<std::vector<std::size_t>, Ant>::const_iterator archive_it;
        for (archive_it = sequential_archive.begin();
             archive_it != sequential_archive.end(); ++archive_it) {
            archived_tours.push_back(archive_it->second);
        }
        // Process the shortest archived tours first.
        sort_ants_by_length(archived_tours);

        for (Ant& ant : archived_tours) {
            const LocalSearchArm selected = select_arm(arms, rng);
            LocalSearchResult local = apply_local_search(selected, ant.tour, graph);
            ant.length = local.after;
            observe_local_search(local, arms, selected, config, result, recent_gains);
            consider_best(ant, result);
            if (config.no_improvement_window > 0 &&
                recent_gains.size() == config.no_improvement_window &&
                no_recent_improvement(recent_gains)) {
                result.stopped_for_no_improvement = true;
                break;
            }
        }
    }

    if (std::isfinite(tau_min) && std::isfinite(tau_max)) {
        result.final_tau_min_bound = tau_min;
        result.final_tau_max_bound = tau_max;
        result.final_pheromone_min = std::numeric_limits<double>::infinity();
        result.final_pheromone_max = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i + 1; j < n; ++j) {
                result.final_pheromone_min = std::min(result.final_pheromone_min, pheromone[i][j]);
                result.final_pheromone_max = std::max(result.final_pheromone_max, pheromone[i][j]);
            }
        }
        if (n < 2) result.final_pheromone_min = result.final_pheromone_max = 0.0;
    }

    for (std::size_t i = 0; i < arms.size(); ++i) {
        result.bandit_posterior_means[i] = arms[i].alpha / (arms[i].alpha + arms[i].beta);
    }
    return result;
}
