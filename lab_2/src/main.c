#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/sysinfo.h>
#include "filters.h"
#include "matrix_utils.h"
#include "thread_utils.h"

void print_usage(const char *program_name) {
    printf("Usage: %s <matrix_size> <K> <max_threads> <enable_output>\n", program_name);
    printf("\nExamples:\n");
    printf("%s 100 5 4 0\n", program_name);
    printf("\nParameters:\n");
    printf("  matrix_size  - size of square matrix (N x N)\n");
    printf("  K            - number of filter applications\n");
    printf("  max_threads  - maximum number of threads\n");
    printf("  enable_output - 1 to display matrices, 0 to disable\n");
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage(argv[0]);
        return 1;
    }
    
    int matrix_size = atoi(argv[1]);
    int K = atoi(argv[2]);
    int max_threads = atoi(argv[3]);
    int enable_output = atoi(argv[4]);
    
    if (matrix_size <= 0 || K <= 0 || max_threads <= 0) {
        printf("Error: all parameters must be positive numbers\n");
        return 1;
    }
    
    printf("=== Execution Parameters ===\n");
    printf("Matrix size: %dx%d\n", matrix_size, matrix_size);
    printf("Number of iterations K: %d\n", K);
    printf("Maximum threads: %d\n", max_threads);
    printf("CPU cores: %d\n", get_nprocs());
    printf("Filter 1: erosion\n");
    printf("Filter 2: dilation\n");
    
    double **input_matrix = create_matrix(matrix_size, matrix_size);
    double **result1 = create_matrix(matrix_size, matrix_size);
    double **result2 = create_matrix(matrix_size, matrix_size);
    
    srand(time(NULL));
    initialize_matrix(input_matrix, matrix_size, matrix_size);
    
    if (enable_output) {
        printf("\nInput matrix (first 10x10):\n");
        print_matrix(input_matrix, 10, 10);
        printf("...\n");
    }
    
    printf("\n=== Starting Processing ===\n");
    double time1 = process_matrix_multithreaded(input_matrix, result1, matrix_size, 
                                               K, max_threads, erosion_filter, "erosion");
    double time2 = process_matrix_multithreaded(input_matrix, result2, matrix_size,
                                               K, max_threads, dilation_filter, "dilation");
    
    printf("\n=== Results ===\n");
    printf("Total execution time: %.4f seconds\n", time1 + time2);
    
    if (enable_output) {
        printf("\nResult erosion (%d iterations, first 10x10):\n", K);
        print_matrix(result1, 10, 10);
        printf("...\n");
        
        printf("\nResult dilation (%d iterations, first 10x10):\n", K);
        print_matrix(result2, 10, 10);
        printf("...\n");
    }
    
    free_matrix(input_matrix, matrix_size);
    free_matrix(result1, matrix_size);
    free_matrix(result2, matrix_size);
    
    printf("\nProcessing completed successfully!\n");
    return 0;
}
