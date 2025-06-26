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

// for a singal cache line
struct Trace {
    char type; //'1' for load, 's' for store
    uint32_t address; // address of the memory access
};

struct CacheLine {
    uint32_t tag = 0;          // tag of the cache line
    bool valid = false;           // valid bit
    bool dirty = false;           // dirty bit for write-back cache
    uint32_t load_time = 0;      // time of the last load operation
    uint32_t access_time = 0;   // for LRU replacement policy
};

struct CacheSet {
    std::vector<CacheLine> lines; // vector of cache lines in the set
    uint32_t lru_counter = 0;      // counter for LRU replacement policy
}

// Cache configuration structure
struct Cache {
    uint32_t num_sets;  // number of sets in the cache
    uint32_t block_num_per_set; // number of blocks per set
    uint32_t block_size;    // size of each block in bytes
    bool is_write_back;     
    bool is_write_allocate;
    bool is_lru;
};

// Cache simulation class
class CacheSim {
public:
    CacheSim(const Cache &config);

    void access_memory(const Trace &trace);
    void print_cache() const;

private:
    Cache config;  // cache configuration
};

// function
bool parse_arguments(int argc, char *argv[], Cache &cache);
bool read_trace_file(const std::string &input, std::vector<Trace> &traces);
void print_cache(const Cache &cache);


#endif // CACHESIM_H