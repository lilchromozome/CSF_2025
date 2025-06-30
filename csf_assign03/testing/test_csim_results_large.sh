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

# Function to compare the output of a command to multiple expected results
compare_output() {
    local command="$1"
    shift
    local expected_outputs=("$@")

    echo "Running: $command"
    output=$(timeout 1m bash -c "$command")
    local cmd_exit_status=$?

    if [ $cmd_exit_status -eq 124 ]; then
        echo "Test Failed for $command: Timed out after 1 minute"
        echo "----------------------------------------"
        return 1
    fi

    local pass=false
    for expected_output in "${expected_outputs[@]}"; do
        # Ensure file exists
        if [[ ! -f "$expected_output" ]]; then
            echo "ERROR: Expected output file $expected_output does not exist"
            continue
        fi

        # Read expected and actual outputs as arrays
        mapfile -t expected_lines < "$expected_output"
        mapfile -t actual_lines <<< "$output"

        # Check if number of lines match
        if [[ ${#expected_lines[@]} -ne ${#actual_lines[@]} ]]; then
            echo "Line count mismatch: Expected ${#expected_lines[@]}, Got ${#actual_lines[@]}"
            continue
        fi

        local match=true
        for i in "${!expected_lines[@]}"; do
            local expected_line="${expected_lines[$i]}"
            local actual_line="${actual_lines[$i]}"

            if [[ "$expected_line" == *"Total cycles:"* ]]; then
                # Handle cycle comparison
                if [[ "$check_cycles" == true ]]; then
                    expected_cycles=$(echo "$expected_line" | grep -oP '(?<=Total cycles: )\d+')
                    actual_cycles=$(echo "$actual_line" | grep -oP '(?<=Total cycles: )\d+')

                    if [ -z "$expected_cycles" ] || [ -z "$actual_cycles" ]; then
                        echo "ERROR: Could not extract cycle counts"
                        match=false
                        break
                    fi

                    # Strict 10% tolerance calculation
                    min_cycles=$(printf "%.0f" $(echo "$expected_cycles * 0.9" | bc -l))
                    max_cycles=$(printf "%.0f" $(echo "$expected_cycles * 1.1" | bc -l))

                    # Strict comparison
                    if [[ $actual_cycles -lt $min_cycles ]] || [[ $actual_cycles -gt $max_cycles ]]; then
                        echo "FAIL: Cycle count out of range"
                        echo "Expected: $expected_cycles (range: $min_cycles - $max_cycles)"
                        echo "Actual:   $actual_cycles"
                        match=false
                        break
                    fi
                fi
            else
                if [ "$expected_line" != "$actual_line" ]; then
                    match=false
                    break
                fi
            fi
        done

        if $match; then
            pass=true
            break
        fi
    done

    if $pass; then
        echo "Test Passed"
        return 0
    else
        echo "Test Failed for $command"
        echo "Actual output:"
        echo "$output"
        return 1
    fi
}

# Define test commands based on replacement policy
if [[ "$replacement_policy" == "lru" ]]; then
    declare -a commands=(
        ".././csim 1 1 4 write-allocate write-through lru <  traces/test_cache2.trace"
        ".././csim 256 4 16 no-write-allocate write-through lru <  traces/test_cache2.trace"
    )
    declare -a expected_outputs=(
        "expected_results/test_cache2/1_1_4_wa_wt_lru.txt"
        "expected_results/test_cache2/256_4_16_nwa_wt_lru.txt"
    )
else
    declare -a commands=(
        ".././csim 256 1 4 no-write-allocate write-through fifo < traces/test_cache2.trace"
        ".././csim 256 4 16 write-allocate write-back fifo <  traces/test_cache2.trace"
    )
    declare -a expected_outputs=(
        "expected_results/test_cache2/256_1_4_nwa_wt_fifo.txt"
        "expected_results/test_cache2/256_4_16_wa_wb_fifo.txt"
    )
fi

# Track test results
total_tests=0
passed_tests=0
failed_tests=0

# Run tests
for i in "${!commands[@]}"; do
    command="${commands[$i]}"
    expected_output="${expected_outputs[$i]}"
    
    # Run the test
    compare_output "$command" "$expected_output"
    result=$?
    
    ((total_tests++))
    if [[ $result -eq 0 ]]; then
        ((passed_tests++))
    else
        ((failed_tests++))
    fi
done

# Print summary
echo "=========================================="
echo "Total Tests: $total_tests"
echo "Passed: $passed_tests"
echo "Failed: $failed_tests"

# Exit with appropriate status
if [[ $failed_tests -eq 0 ]]; then
    echo "All tests passed successfully!"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi