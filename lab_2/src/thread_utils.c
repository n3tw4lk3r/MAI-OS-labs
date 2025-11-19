#include "thread_utils.h"
#include "matrix_utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <stdio.h>

double apply_filter(double **matrix, size_t row, size_t column, size_t number_of_rows,
                   size_t number_of_columns, filter_function_t filter_function) {
    return filter_function(matrix, row, column, number_of_rows, number_of_columns);
}

void* process_matrix_part(void *argument) {
    thread_data_t *data = (thread_data_t *)argument;
    
    for (size_t iteration = 0; iteration < data->k; ++iteration) {
        double **current_matrix;
        if (iteration == 0)
            current_matrix = data->input_matrix;
        else
            current_matrix = data->output_matrix;
        double **temp_matrix = data->output_matrix;
        
        for (size_t i = data->start_row; i < data->end_row; ++i) {
            for (size_t j = 0; j < data->number_of_columns; ++j) {
                temp_matrix[i][j] = apply_filter(current_matrix, i, j, 
                                               data->end_row, data->number_of_columns, data->filter_func);
            }
            
            if (pthread_mutex_lock(data->mutex) != 0) {
                const char* error_msg = "Error: pthread_mutex_lock failed\n";
                write(STDERR_FILENO, error_msg, strlen(error_msg));
                return NULL;
            }
            (*data->completed_rows)++;
            if (pthread_mutex_unlock(data->mutex) != 0) {
                const char* error_msg = "Error: pthread_mutex_unlock failed\n";
                write(STDERR_FILENO, error_msg, strlen(error_msg));
                return NULL;
            }
        }
        
        if (iteration < data->k - 1) {
            for (size_t i = data->start_row; i < data->end_row; ++i) {
                for (size_t j = 0; j < data->number_of_columns; ++j) {
                    current_matrix[i][j] = temp_matrix[i][j];
                }
            }
        }
    }
    
    return NULL;
}

double process_matrix_multithreaded(double **input_matrix, double **output_matrix,
                                   size_t matrix_size, size_t k, size_t max_number_of_threads,
                                   filter_function_t filter_function, const char* filter_name) {
    clock_t start_time = clock();
    if (start_time == (clock_t)-1) {
        const char* error_msg = "Error: clock() failed\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return -1.0;
    }
    
    size_t actual_threads;
    if (max_number_of_threads < matrix_size)
        actual_threads = max_number_of_threads;
    else
        actual_threads = matrix_size;

    size_t rows_per_thread = matrix_size / actual_threads;
    size_t extra_rows = matrix_size % actual_threads;
    
    pthread_t *threads = (pthread_t*) malloc(actual_threads * sizeof(pthread_t));
    if (threads == NULL) {
        const char* error_msg = "Error: malloc failed for threads\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return -1.0;
    }
    
    thread_data_t *thread_data = (thread_data_t*) malloc(actual_threads * sizeof(thread_data_t));
    if (thread_data == NULL) {
        const char* error_msg = "Error: malloc failed for thread_data\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        free(threads);
        return -1.0;
    }
    
    pthread_mutex_t mutex;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        const char* error_msg = "Error: pthread_mutex_init failed\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        free(threads);
        free(thread_data);
        return -1.0;
    }
    
    size_t completed_rows = 0;
    
    double **current_input = create_matrix(matrix_size, matrix_size);
    if (current_input == NULL) {
        const char* error_msg = "Error: create_matrix failed\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        pthread_mutex_destroy(&mutex);
        free(threads);
        free(thread_data);
        return -1.0;
    }
    
    copy_matrix(input_matrix, current_input, matrix_size, matrix_size);
    copy_matrix(input_matrix, output_matrix, matrix_size, matrix_size);
    
    size_t current_start = 0;
    for (size_t i = 0; i < actual_threads; ++i) {
        thread_data[i].input_matrix = current_input;
        thread_data[i].output_matrix = output_matrix;
        thread_data[i].start_row = current_start;
        thread_data[i].end_row = current_start + rows_per_thread + (i < extra_rows ? 1 : 0);
        thread_data[i].number_of_columns = matrix_size;
        thread_data[i].filter_func = filter_function;
        thread_data[i].k = k;
        thread_data[i].mutex = &mutex;
        thread_data[i].completed_rows = &completed_rows;
        
        current_start = thread_data[i].end_row;
        
        if (pthread_create(&threads[i], NULL, process_matrix_part, &thread_data[i]) != 0) {
            const char* error_msg = "Error: pthread_create failed\n";
            write(STDERR_FILENO, error_msg, strlen(error_msg));
            
            for (size_t j = 0; j < i; ++j) {
                pthread_join(threads[j], NULL);
            }
            pthread_mutex_destroy(&mutex);
            free(threads);
            free(thread_data);
            free_matrix(current_input, matrix_size);
            return -1.0;
        }
    }
    
    size_t last_completed = 0;
    while (completed_rows < matrix_size * k) {
        if (completed_rows > last_completed) {
            char progress_msg[256];
            int len = snprintf(progress_msg, sizeof(progress_msg), 
                             "Progress %s: %zu/%zu rows processed\r", 
                             filter_name, completed_rows, matrix_size * k);
            if (len > 0) {
                write(STDOUT_FILENO, progress_msg, (size_t)len);
            }
            last_completed = completed_rows;
        }
        if (usleep(100000) != 0) {
            const char* error_msg = "Error: usleep failed\n";
            write(STDERR_FILENO, error_msg, strlen(error_msg));
        }
    }
    write(STDOUT_FILENO, "\n", 1);
    
    for (size_t i = 0; i < actual_threads; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            const char* error_msg = "Error: pthread_join failed\n";
            write(STDERR_FILENO, error_msg, strlen(error_msg));
        }
    }
    
    if (pthread_mutex_destroy(&mutex) != 0) {
        const char* error_msg = "Error: pthread_mutex_destroy failed\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
    }
    
    free(threads);
    free(thread_data);
    free_matrix(current_input, matrix_size);
    
    clock_t end_time = clock();
    if (end_time == (clock_t)-1) {
        const char* error_msg = "Error: clock() failed\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return -1.0;
    }
    
    double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    char time_msg[BUFSIZ];
    int time_len = snprintf(time_msg, sizeof(time_msg), 
                          "Execution time %s: %.4f seconds\n", 
                          filter_name, execution_time);
    if (time_len > 0) {
        write(STDOUT_FILENO, time_msg, (size_t)time_len);
    }
    
    char threads_msg[BUFSIZ];
    int threads_len = snprintf(threads_msg, sizeof(threads_msg), 
                             "Threads used: %zu\n", actual_threads);
    if (threads_len > 0) {
        write(STDOUT_FILENO, threads_msg, (size_t)threads_len);
    }
    
    return execution_time;
}
