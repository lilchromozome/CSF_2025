import subprocess
import csv
from itertools import product

# Generate all combinations of parameters
num_sets = [2**i for i in range(0, 13)]       # 1 to 4096
block_num_per_set = [2**i for i in range(0, 9)]         # 1 to 256
block_size = [2**i for i in range(2, 6)]              # 4 to 32
write_allocs = ["write-allocate", "no-write-allocate"]
write_policies = ["write-back", "write-through"]
replacements = ["fifo", "lru"]

# Output CSV file
with open("cache_sim_results.csv", "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    # Write header
    writer.writerow([
        "CacheSize", "BlockSize", "Associativity", "WriteAlloc", "WritePolicy", "Replacement",
        "LoadMisses", "StoreMisses", "TotalCycles", "Command"
    ])
    
    # Counter for progress monitoring
    total_combinations = len(num_sets) * len(block_num_per_set) * len(block_size) * 8  # 8 policy combinations
    current = 0
    
    # Iterate through all combinations
    for n_sets, bin_set, b_size, walloc, wpolicy, repl in product(
        num_sets, block_num_per_set, block_size, write_allocs, write_policies, replacements
    ):
        # Skip invalid combinations
        if n_sets < bin_set * b_size or n_sets % (bin_set * b_size) != 0:
            continue
            
        current += 1
        print(f"Running {current}/{total_combinations}: {n_sets},{bin_set},{b_size},{walloc},{wpolicy},{repl}")
        
        # Build command
        cmd = f"./csim {n_sets} {bin_set} {b_size} {walloc} {wpolicy} {repl} < gcc.trace"
        
        try:
            # Run command and capture output
            result = subprocess.run(
                cmd, 
                shell=True, 
                check=True, 
                text=True, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE
            )
            
            # Parse output
            output = result.stdout
            metrics = {}
            for line in output.splitlines():
                if "Load misses" in line:
                    metrics["load_misses"] = int(line.split(":")[1].strip())
                elif "Store misses" in line:
                    metrics["store_misses"] = int(line.split(":")[1].strip())
                elif "Total cycles" in line:
                    metrics["total_cycles"] = int(line.split(":")[1].strip())
            
            # Write results to CSV
            writer.writerow([
                n_sets, bin_set, b_size, walloc, wpolicy, repl,
                metrics.get("load_misses", ""),
                metrics.get("store_misses", ""),
                metrics.get("total_cycles", ""),
                cmd
            ])
            
        except subprocess.CalledProcessError as e:
            print(f"Error running command: {cmd}")
            print(f"Error message: {e.stderr}")
            # Write error placeholder
            writer.writerow([
                n_sets, bin_set, b_size, walloc, wpolicy, repl,
                "ERROR", "ERROR", "ERROR", cmd
            ])
            
        # Flush after each run to preserve progress
        csvfile.flush()

print("Simulation complete! Results saved to cache_sim_results.csv")