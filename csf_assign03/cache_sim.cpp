#include "cache_sim.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>

// Cache sim constructor
CacheSim::CacheSim(const Cache &cfg)
    : config(cfg)
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
    CacheSet &st = sets[set_idx];
    bool hit = find_cache_line(st, tag);

    if (load)
        handle_load(st, tag, hit);
    else
        handle_store(st, tag, hit);
}

std::pair<uint32_t, uint32_t> CacheSim::parse_address(uint32_t addr) {
    uint32_t boffs = __builtin_ctz(config.block_size);
    uint32_t soffs = __builtin_ctz(config.num_sets);
    uint32_t set_idx = (addr >> boffs) & ((1u << soffs) - 1);
    uint32_t tag = addr >> (boffs + soffs);
    return {set_idx, tag};
}

bool CacheSim::find_cache_line(CacheSet &st, uint32_t tag) {
    for (auto &line : st.lines) {
        if (line.valid && line.tag == tag) {
            if (config.is_lru) line.access_time = st.lru_counter++;
            return true;
        }
    }
    return false;
}

CacheLine* CacheSim::evict_line(CacheSet &st) {
    for (auto &ln : st.lines)
        if (!ln.valid) return &ln;

    CacheLine *ev = &*std::min_element(st.lines.begin(), st.lines.end(),
        [](auto &a, auto &b){ return a.access_time < b.access_time; });

    if (ev->dirty && config.is_write_back)
        total_cycles += (config.block_size / 4) * 100;

    return ev;
}

void CacheSim::handle_load(CacheSet &st, uint32_t tag, bool hit) {
    if (hit) {
        load_hits++; total_cycles++;
    } else {
        load_misses++;
        total_cycles += 1 + (config.block_size / 4) * 100;
        CacheLine *ev = evict_line(st);
        *ev = {tag, true, false, 0, st.lru_counter++};
    }
}

void CacheSim::handle_store(CacheSet &st, uint32_t tag, bool hit) {
    if (hit) {
        store_hits++; total_cycles++;
        for (auto &ln : st.lines)
            if (ln.valid && ln.tag == tag) {
                if (config.is_write_back) ln.dirty = true;
                else total_cycles += 100;
                break;
            }
    } else {
        store_misses++;
        if (config.is_write_allocate) {
            total_cycles += 1 + (config.block_size / 4) * 100;
            CacheLine *ev = evict_line(st);
            *ev = {tag, true, config.is_write_back, 0, st.lru_counter++};
            if (!config.is_write_back) total_cycles += 100;
        } else {
            total_cycles += 100;
        }
    }
}
