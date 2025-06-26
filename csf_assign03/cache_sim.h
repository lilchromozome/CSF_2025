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
    uint32_t address // address of the memory access
}

// Cache class
struct Cache {
    uint32_t num_sets;  // number of sets in the cache
    uint32_t block_num_per_set; // number of blocks per set
    uint32_t block_size;    // size of each block in bytes
    bool is_write_back;     
    bool is_write_allocate;
    bool is_lru;
}

// function
bool parse_arguments(int argc, char *argv[], Cache &cache);
bool read_trace_file(const std::string &input, std::vector<Trace> &traces);
void print_cache(const Cache &cache);


#endif // CACHESIM_H