#include "cache_sim.h"

static inline bool is_power_of_two(uint32_t x) { return x && !(x & (x - 1)); }

/*
    * Parese and validate command line arguements to populate the cache settings
    *
    * Parameters:
    *   argc - number of command line arguments
    *   argv - arugument values
    *   cache - a Cache object to be populated with the parsed values
    *
    * Returns:
    *   true if the arguments are valid and parsed successfully, false otherwise.
*/
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
    cache.is_lru  = (repl == "lru");
    cache.is_fifo = (repl == "fifo");
    if (!(cache.is_lru ^ cache.is_fifo)) {
         return false;
    }


    if (!is_power_of_two(cache.num_sets) || !is_power_of_two(cache.block_num_per_set)
        || !is_power_of_two(cache.block_size) || cache.block_size < 4
        || (!cache.is_write_allocate && cache.is_write_back)) {

        std::cerr << "Invalid cache parameters\n";
        return false;
    }
    return true;
}


/*
    *Read trace data from the input stream and populate the traces vector.
    *
    * Parameters:
    *   input - input stream containing the trace data
    *   traces - vector to be populated with Trace objects
    *
    * Returns:
    *   true if the trace file is read successfully, false otherwise.
*/
bool read_trace_file(std::istream &input, std::vector<Trace> &traces) {
    char type;
    std::string addr_str;
    uint32_t sz;

    while (input >> type >> addr_str >> sz) {
        Trace t{type, static_cast<uint32_t>(std::stoul(addr_str, nullptr, 16))};
        traces.push_back(t);
    }
    return true;
}

int main(int argc, char *argv[]) {
    Cache cache;
    if (!parse_arguments(argc, argv, cache)) {
        std::cerr << "Incorrect input format " << std::endl;
        return 1;
    }

    std::vector<Trace> traces;
    if (!read_trace_file(std::cin, traces)) {
        std::cerr << "Error in trace file" << std::endl;
        return 1;
    }

    CacheSim sim(cache);
    for (const auto &t : traces) sim.access_memory(t);
    sim.print_cache();

    return 0;
}
