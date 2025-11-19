#pragma once

#include <pthread.h>
#include "filters.h"
#include "matrix_utils.h"

typedef struct {
    double **input_matrix;
    double **output_matrix;
    size_t start_row;
    size_t end_row;
    size_t number_of_columns;
    filter_function_t filter_func;
    size_t k;
    pthread_mutex_t *mutex;
    size_t *completed_rows;
} thread_data_t;

double apply_filter(double **matrix, size_t row, size_t column, size_t number_of_rows,
                   size_t number_of_columns, filter_function_t filter_function);
void* process_matrix_part(void *arg);
double process_matrix_multithreaded(double **input_matrix, double **output_matrix,
                                   size_t matrix_size, size_t k, size_t max_number_of_threads,
                                   filter_function_t filter_function, const char* filter_name);
