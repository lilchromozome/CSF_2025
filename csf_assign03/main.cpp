#include "cache_sim.h"

int main(int argc, char *argv[]) {
    Cache cache;
    if(parse_arguments(argc, argv, cache)){
        return 0;
    }
    return 1;
}
