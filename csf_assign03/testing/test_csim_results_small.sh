#!/bin/bash

# Get the directory where this script is located
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
parent_dir="$(dirname "$script_dir")"
csim_exec="$parent_dir/csim"

# Ensure csim exists
if [[ ! -x "$csim_exec" ]]; then
    echo "Error: csim executable not found in $parent_dir or not executable."
    exit 1
fi

# Check arguments
if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 [LRU|FIFO] [cycles]"
    echo "  - First argument: Test only LRU or FIFO implementation"
    echo "  - Second argument (optional): If 'cycles' is provided, allow cycle counts to vary by 10%"
    exit 1
fi

# Set replacement policy to test
replacement_policy=$(echo "$1" | tr '[:upper:]' '[:lower:]')
if [[ "$replacement_policy" != "lru" && "$replacement_policy" != "fifo" ]]; then
    echo "Error: First argument must be either LRU or FIFO"
    exit 1
fi

# Enable cycle variation checking if "cycles" is the second argument
check_cycles=false
if [[ "$2" == "cycles" ]]; then
    check_cycles=true
fi

# Define test cases
declare -A commands
declare -A expected_outputs
declare -A command_labels

trace_files=("read01" "read02" "read03" "write01" "write02")
configs=(
    "1 1 4 write-allocate write-through"
    "1 1024 128 no-write-allocate write-through"
    "1 1024 128 write-allocate write-through"
    "1 1024 128 write-allocate write-back"
    "8192 1 16 no-write-allocate write-through"
    "8192 1 16 write-allocate write-through"
    "8192 1 16 write-allocate write-back"
    "256 16 64 no-write-allocate write-through"
    "128 32 64 write-allocate write-through"
    "2048 4 16 no-write-allocate write-through"
    "2048 4 16 write-allocate write-through"
    "2048 4 16 write-allocate write-back"
    "1024 1 128 write-allocate write-through"
    "1024 1 128 write-allocate write-back"
    "256 4 128 no-write-allocate write-through"
    "256 4 128 write-allocate write-through"
    "256 4 128 write-allocate write-back"
)

# Tracking test results
total_tests=0
passed_tests=0
failed_tests=0

# Generate test cases
cmd_index=1
for trace in "${trace_files[@]}"; do
    trace_file="$script_dir/traces/$trace.trace"
    
    if [[ ! -f "$trace_file" ]]; then
        echo "ERROR: Missing trace file: $trace_file"
        continue
    fi

    expected_dir="$script_dir/expected_results/$trace"
    
    if [[ ! -d "$expected_dir" ]]; then
        echo "ERROR: Expected results directory does not exist: $expected_dir"
        continue
    fi

    for config in "${configs[@]}"; do
        read -ra config_parts <<< "$config"
        
        set_size="${config_parts[0]}"
        assoc="${config_parts[1]}"
        block_size="${config_parts[2]}"
        write_alloc="${config_parts[3]}"
        write_policy="${config_parts[4]}"
        
        write_alloc_flag="wa"
        [[ "$write_alloc" == "no-write-allocate" ]] && write_alloc_flag="nwa"

        write_policy_flag="wt"
        [[ "$write_policy" == "write-back" ]] && write_policy_flag="wb"

        expected_filename="${set_size}_${assoc}_${block_size}_${write_alloc_flag}_${write_policy_flag}_${replacement_policy}.txt"
        expected_output="$expected_dir/$expected_filename"

        if [[ ! -f "$expected_output" ]]; then
            echo "ERROR: Expected result file missing: $expected_output"
            continue
        fi

        formatted_command="csim $set_size $assoc $block_size $write_alloc $write_policy $replacement_policy < $trace.trace"
        commands["cmd$cmd_index"]="$csim_exec $set_size $assoc $block_size $write_alloc $write_policy $replacement_policy < $trace_file"
        expected_outputs["cmd$cmd_index"]="$expected_output"
        command_labels["cmd$cmd_index"]="$formatted_command"
        ((cmd_index++))
    done
done

# Function to compare output with expected result
compare_output() {
    local command="$1"
    local expected_output="$2"
    local formatted_command="$3"

    echo "$formatted_command"
    echo "-----------------------------------------------------------------------"

    output_file=$(mktemp)
    eval "$command" > "$output_file"

    ((total_tests++))

    if [[ ! -s "$output_file" ]]; then
        echo "ERROR: csim produced empty output"
        ((failed_tests++))
        return
    fi

    # Extract cycle information
    expected_cycles=$(grep -oP '(?<=Total cycles: )\d+' "$expected_output")
    actual_cycles=$(grep -oP '(?<=Total cycles: )\d+' "$output_file")

    # Decide how to handle cycle comparison based on command-line argument
    if $check_cycles && [[ -n "$expected_cycles" && -n "$actual_cycles" ]]; then
        # Calculate tolerance range
        min_cycles=$(awk "BEGIN {print $expected_cycles * 0.9}")
        max_cycles=$(awk "BEGIN {print $expected_cycles * 1.1}")

        # Check if actual cycles are within 10% of expected
        if (( $(awk "BEGIN {print ($actual_cycles >= $min_cycles && $actual_cycles <= $max_cycles)}") )); then
            echo "Test Passed (Cycles within ±10%)"
            ((passed_tests++))
            rm -f "$output_file"
            return
        else
            echo "Test Failed (Cycles out of range)"
            echo "Expected: $expected_cycles ±10% ($min_cycles - $max_cycles)"
            echo "Actual: $actual_cycles"
            ((failed_tests++))
            rm -f "$output_file"
            return
        fi
    fi

    # Compare the rest of the output line by line
    head -n -1 "$expected_output" > expected_trimmed.txt
    head -n -1 "$output_file" > output_trimmed.txt
    diff -w expected_trimmed.txt output_trimmed.txt > /dev/null

    if [[ $? -eq 0 ]]; then
        echo "Test Passed"
        ((passed_tests++))
    else
        echo "Test Failed"
        echo "Differences:"
        diff -w expected_trimmed.txt output_trimmed.txt | head -20
        ((failed_tests++))
    fi

    rm -f "$output_file" expected_trimmed.txt output_trimmed.txt
}

# Run tests
for cmd_key in "${!commands[@]}"; do
    compare_output "${commands[$cmd_key]}" "${expected_outputs[$cmd_key]}" "${command_labels[$cmd_key]}"
done

# Print final test summary
echo "=========================================="
echo "Total Tests Run: $total_tests"
echo "Passed: $passed_tests"
echo "Failed: $failed_tests"

if [[ $failed_tests -eq 0 ]]; then
    echo "All tests passed successfully!"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi