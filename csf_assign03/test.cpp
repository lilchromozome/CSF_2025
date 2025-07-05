#include "cache_sim.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <algorithm>

struct ResultOutput {
    uint32_t sets_num;              //sets number in cache
    uint32_t block_num_per_set;     //block number in each set
    uint32_t block_size;            //block size
    bool write_allocate;            //is allocate block when write
    bool write_back;                //is write back
    bool LRU;                       //is LRU, or FIFO
    uint64_t cycles;                //total cycles
    double miss_rate_load;          //miss rate of load
    double miss_rate_store;         //miss rate of store
    double totle_score;             //total score, works better    
};

double calculate_score(const ResultOutput& r) {
    const double weight_cycle = 0.6;
    const double weight_miss = 0.2;
    return weight_cycle * (r.cycles * 1e-10) +
        weight_miss * (r.miss_rate_load * 100) +
        weight_miss * (r.miss_rate_store * 100);
}

ResultOutput run_one_test(const std::string& trace_file, const Cache& config) {
    CacheSim sim(config);
    std::ifstream infile(trace_file);
    std::string line;

    // process eac memory access in the trace file
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        char type;
        uint32_t address;
        iss >> type >> std::hex >> address;
        sim.access_memory({type, address});
    }

    ResultOutput result;
    result.sets_num = config.num_sets;
    result.block_num_per_set = config.block_num_per_set;
    result.block_size = config.block_size;
    result.write_allocate = config.is_write_allocate;
    result.write_back = config.is_write_back;
    result.LRU = !config.is_fifo;
    result.cycles = sim.get_total_cycles();
    result.miss_rate_load = static_cast<double>(sim.get_load_misses()) /
                        std::max(static_cast<uint64_t>(1), sim.get_total_loads());

    result.miss_rate_store = static_cast<double>(sim.get_store_misses()) /
                         std::max(static_cast<uint64_t>(1), sim.get_total_stores());


    result.totle_score = calculate_score(result);

    return result;
}

void print_result(const ResultOutput& r) {
    std::cout << "Config: "
              << r.sets_num << " sets, "
              << r.block_num_per_set << " blocks/set, "
              << r.block_size << "B, "
              << (r.write_allocate ? "write-allocate, " : "no-write-allocate, ")
              << (r.write_back ? "write-back, " : "write-through, ")
              << (r.LRU ? "LRU" : "FIFO") << std::endl;
    std::cout << "Cycles: " << r.cycles
              << ", Load Miss Rate: " << r.miss_rate_load
              << ", Store Miss Rate: " << r.miss_rate_store
              << ", Score: " << r.totle_score << std::endl << std::endl;
}

int main() {
    std::string trace_file = "gcc.trace";  

    std::vector<ResultOutput> all_results;

    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> configs = {
        {128, 4, 16}, {256, 2, 16}, {512, 1, 16},
        {64, 4, 32}, {128, 2, 32}, {256, 1, 32}
    };

    std::vector<std::pair<bool, bool>> write_policies = {
        {true, true}, {true, false}, {false, false}
    };

    std::vector<bool> lru_options = {true, false};

    for (auto [sets, blocks, size] : configs) {
        for (auto [wa, wb] : write_policies) {
            for (bool is_lru : lru_options) {
                Cache config{sets, blocks, size, wa, wb, is_lru, !is_lru};
                ResultOutput result = run_one_test(trace_file, config);
                all_results.push_back(result);
            }
        }
    }

    // sort to find the lowest score
    std::sort(all_results.begin(), all_results.end(), [](const ResultOutput& a, const ResultOutput& b) {
        return a.totle_score < b.totle_score;
    });

    for (const auto& r : all_results) {
        print_result(r);
    }

    std::cout << "Best configuration:\n";
    print_result(all_results.front());

    return 0;
}

