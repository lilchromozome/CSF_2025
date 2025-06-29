#include "cache_sim.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>

static inline bool is_power_of_two(uint32_t x) { return x && !(x & (x - 1)); }

bool parse_arguments(int argc, char *argv[], Cache &cache) {
    if (argc != 7) return false;

    cache.num_sets = std::stoul(argv[1]);           // number of sets in the cache (a positive power-of-2)
    cache.block_num_per_set = std::stoul(argv[2]);  //number of blocks in each set (a positive power-of-2)
    cache.block_size = std::stoul(argv[3]);         // number of bytes in each block (a positive power-of-2, at least 4)
    std::string wa = argv[4];                       // write-through or write-back
    std::string wb = argv[5];                       // write-allocate or no-write-allocate
    std::string repl = argv[6];                     // lru (least-recently-used) or fifo evictions

    cache.is_write_allocate = (wa == "write-allocate");
    cache.is_write_back = (wb == "write-back");
    cache.is_lru = (repl == "lru");

    if (!is_power_of_two(cache.num_sets) || !is_power_of_two(cache.block_num_per_set)
        || !is_power_of_two(cache.block_size) || cache.block_size < 4
        || (!cache.is_write_allocate && cache.is_write_back)) {

        std::cerr << "Invalid cache parameters\n";
        return false;
    }
    return true;
}

