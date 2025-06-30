#ifndef CACHESIM_H
#define CACHESIM_H

#include <initializer_list>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <algorithm>


// for a single cache line
struct Trace {
    char type; //'1' for load, 's' for store
    uint32_t address; 
};

struct CacheLine {
    uint32_t tag = 0;          // cache line tag
    bool valid = false;           // valid bit
    bool dirty = false;           // for write-back 
    uint32_t load_time = 0;      // time of the last load operation
    uint32_t access_time = 0;   // for LRU replacement 
};

struct CacheSet {
    std::vector<CacheLine> lines; 
    uint32_t lru_counter = 0;      // counter for LRU replacement 
};

// Cache configuration structure
struct Cache {
    uint32_t num_sets;          // number of sets in the cache (a positive power-of-2)
    uint32_t block_num_per_set; // number of blocks in each set (a positive power-of-2)
    uint32_t block_size;        // number of bytes in each block (a positive power-of-2, at least 4)
    bool is_write_allocate;     // write-allocate or no-write-allocate
    bool is_write_back;         // write-through or write-back
    bool is_lru;                // lru (least-recently-used) or fifo evictions
};

// Cache simulation class
class CacheSim {
public:
    CacheSim(const Cache &config);

    void access_memory(const Trace &trace);
    void print_cache() const;

    //helper fxns
    std::pair<uint32_t, uint32_t> parse_address(uint32_t addr);
    CacheLine* remove_line(CacheSet &st);                   //lazy
    bool find_cache_line(CacheSet &st, uint32_t tag);
    void handle_load(CacheSet &st, uint32_t tag, bool hit);
    void handle_store(CacheSet &st, uint32_t tag, bool hit);


private:
    Cache config;  // cache configuration
    std::vector<CacheSet> sets;  

    // Your cache simulator should assume that loads/stores from/to the cache take one processor cycle; 
    // loads/stores from/to memory take 100 processor cycles for each 4-byte quantity that is transferred. 
    uint64_t total_loads = 0;
    uint64_t total_stores = 0;
    uint64_t load_hits = 0;
    uint64_t load_misses = 0;
    uint64_t store_hits = 0;
    uint64_t store_misses = 0;
    uint64_t total_cycles = 0;
};

// for main
bool parse_arguments(int argc, char *argv[], Cache &cache);
bool read_trace_file(const std::string &input, std::vector<Trace> &traces);
void print_cache(const Cache &cache);


#endif // CACHESIM_H