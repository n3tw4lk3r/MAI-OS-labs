#!/bin/bash

echo "=== Accurate Performance Testing ==="

echo "Building project..."
mkdir -p build
cd build
cmake ..
make -j4
cd ..

LOGICAL_CORES=$(nproc)
echo "Logical cores: $LOGICAL_CORES"

MATRIX_SIZES=(1000 2000 3000)
THREAD_COUNTS=(1 2 $LOGICAL_CORES 8 16)
K_ITERATIONS=3

echo "Testing matrix sizes: ${MATRIX_SIZES[@]}"
echo "Testing thread counts: ${THREAD_COUNTS[@]}"
echo "K iterations: $K_ITERATIONS"
echo ""

RESULTS_FILE="performance_results.txt"

echo "Performance Results Table" > $RESULTS_FILE
echo "=========================" >> $RESULTS_FILE
echo "Matrix Size | Threads | Time (s) | Speedup | Efficiency" >> $RESULTS_FILE
echo "------------|---------|----------|---------|-----------" >> $RESULTS_FILE

for size in "${MATRIX_SIZES[@]}"; do
    echo "--- Testing matrix: ${size}x${size} ---"

    BASE_TIME=0

    for threads in "${THREAD_COUNTS[@]}"; do
        echo -n "  Threads: $threads ... "

        START_TIME=$(date +%s.%N)
        ./build/lab_2 $size $K_ITERATIONS $threads 0 > /dev/null 2>&1
        EXIT_CODE=$?
        END_TIME=$(date +%s.%N)

        if [ $EXIT_CODE -ne 0 ]; then
            echo "FAILED (exit code: $EXIT_CODE)"
            printf "%-11s | %-7s | %-8s | %-7s | %-9s\n" \
                   "${size}x${size}" "$threads" "FAILED" "N/A" "N/A" >> $RESULTS_FILE
            continue
        fi

        TIME_TOTAL=$(echo "$END_TIME - $START_TIME" | bc -l)

        if [ $threads -eq 1 ]; then
            BASE_TIME=$TIME_TOTAL
        fi

        if (( $(echo "$BASE_TIME > 0 && $TIME_TOTAL > 0" | bc -l) )); then
            SPEEDUP=$(echo "scale=2; $BASE_TIME / $TIME_TOTAL" | bc -l)
            EFFICIENCY=$(echo "scale=2; $SPEEDUP / $threads * 100" | bc -l)
        else
            SPEEDUP="1.00"
            EFFICIENCY="100.00"
        fi

        printf "%-11s | %-7s | %-8.3f | %-7s | %-9s\n" \
               "${size}x${size}" "$threads" "$TIME_TOTAL" "$SPEEDUP" "${EFFICIENCY}%" >> $RESULTS_FILE

        echo "Time: ${TIME_TOTAL}s, Speedup: ${SPEEDUP}x, Efficiency: ${EFFICIENCY}%"
    done
    echo ""
done

echo ""
echo "=== Results Summary ==="
cat $RESULTS_FILE
