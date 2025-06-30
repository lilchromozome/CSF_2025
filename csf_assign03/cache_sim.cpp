#include "cache_sim.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>

// Cache sim constructor
CacheSim::CacheSim(const Cache &config)
    : config(config)
{
    // resize sets
    sets.resize(config.num_sets);
    for (auto &s : sets) {
        s.lines.resize(config.block_num_per_set);
        s.lru_counter = 0;
    }
}

void CacheSim::access_memory(const Trace &trace) {
    bool load = (trace.type == 'l');
    if (load) total_loads++; else total_stores++;

    auto [set_idx, tag] = parse_address(trace.address);
    CacheSet &cache_set = sets[set_idx];
    bool hit = find_cache_line(cache_set, tag);

    if (load)
        handle_load(cache_set, tag, hit);
    else
        handle_store(cache_set, tag, hit);
}

std::pair<uint32_t, uint32_t> CacheSim::parse_address(uint32_t addr) {
    uint32_t block_offsets = __builtin_ctz(config.block_size);                  // set/block is pow 2
    uint32_t set_offsets = __builtin_ctz(config.num_sets);
    uint32_t set_idx = (addr >> block_offsets) & ((1u << set_offsets) - 1);     // size blocks * sets = index
    uint32_t tag = addr >> (block_offsets + set_offsets);                       // remainder = tag
    return {set_idx, tag};
}

bool CacheSim::find_cache_line(CacheSet &cache_set, uint32_t tag) {
    for (auto &line : cache_set.lines) {
        if (line.valid && line.tag == tag) {
            // hit
            if (config.is_lru) line.access_time = cache_set.lru_counter++;
            return true;
        }
    }
    // miss
    return false;
}

CacheLine* CacheSim::remove_line(CacheSet &cache_set) {
    for (auto &ln : cache_set.lines)
        if (!ln.valid) return &ln;

    CacheLine *remove_me = &*std::min_element(cache_set.lines.begin(), cache_set.lines.end(),
        [](auto &a, auto &b){ return a.access_time < b.access_time; });

    if (remove_me->dirty && config.is_write_back)
        total_cycles += (config.block_size / 4) * 100;

    return remove_me;
}

/*
Read
*/
void CacheSim::handle_load(CacheSet &cache_set, uint32_t tag, bool hit) {
    if (hit) {
        load_hits++; total_cycles++;
    } else {
        load_misses++;
        total_cycles += 1 + (config.block_size / 4) * 100;
        CacheLine *remove_me = remove_line(cache_set);
        *remove_me = {tag, true, false, 0, cache_set.lru_counter++};
    }
}

/*
Write
*/
void CacheSim::handle_store(CacheSet &cache_set, uint32_t tag, bool hit) {
    if (hit) {
        store_hits++; total_cycles++;
        for (auto &ln : cache_set.lines)
            if (ln.valid && ln.tag == tag) {
                if (config.is_write_back) ln.dirty = true;
                else total_cycles += 100;
                break;
            }
    } else {
        store_misses++;
        if (config.is_write_allocate) {
            total_cycles += 1 + (config.block_size / 4) * 100;
            CacheLine *remove_me = remove_line(cache_set);
            *remove_me = {tag, true, config.is_write_back, 0, cache_set.lru_counter++};
            if (!config.is_write_back) total_cycles += 100;
        } else {
            total_cycles += 100;
        }
    }
}

void CacheSim::print_cache() const{
    std::cout << "Total loads: " << total_loads << std::endl;
    std::cout << "Total stores: " << total_stores << std::endl;
    std::cout << "Load hits: " << load_hits << std::endl;
    std::cout << "Load misses: " << load_misses << std::endl;
    std::cout << "Store hits: " << store_hits << std::endl;
    std::cout << "Store misses: " << store_misses << std::endl;
    std::cout << "Total cycles: " << total_cycles << std::endl;
}

// // debug
// void CacheSim::print_cache() const {
//     for (size_t i = 0; i < sets.size(); ++i) {
//         std::cout << "Set " << i << ":\n";
//         for (auto &ln : sets[i].lines) {
//             std::cout << " tag=" << std::hex << ln.tag << std::dec
//                       << " v=" << ln.valid << " d=" << ln.dirty
//                       << " at=" << ln.access_time << "\n";
//         }
//     }
// }

// void print_cache(const Cache &cache) {
//     std::cout << "Cache: " << cache.num_sets << " sets, "
//               << cache.block_num_per_set << " ways, block size "
//               << cache.block_size << " bytes\n";
// }