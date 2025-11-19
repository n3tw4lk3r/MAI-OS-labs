#include "matrix_utils.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

double** create_matrix(size_t number_of_rows, size_t number_of_columns) {
    double **matrix = (double**) malloc(number_of_rows * sizeof(double*));
    if (matrix == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < number_of_rows; ++i) {
        matrix[i] = (double*) malloc(number_of_columns * sizeof(double));
        if (matrix[i] == NULL) {
            for (size_t j = 0; j < i; ++j) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }
    return matrix;
}

void free_matrix(double **matrix, size_t number_of_rows) {
    if (matrix == NULL) return;

    for (size_t i = 0; i < number_of_rows; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

void initialize_matrix(double **matrix, size_t number_of_rows,
                       size_t number_of_columns) {
    for (size_t i = 0; i < number_of_rows; ++i) {
        for (size_t j = 0; j < number_of_columns; ++j) {
            matrix[i][j] = (double)rand() / RAND_MAX * 100.0;
        }
    }
}

void copy_matrix(double **source, double **destination, size_t number_of_rows,
                 size_t number_of_columns) {
    for (size_t i = 0; i < number_of_rows; ++i) {
        for (size_t j = 0; j < number_of_columns; ++j) {
            destination[i][j] = source[i][j];
        }
    }
}

void print_matrix(double **matrix, size_t number_of_rows, size_t number_of_columns) {
    char buffer[BUFSIZ];
    const char newline = '\n';
    const char space = ' ';

    for (size_t i = 0; i < number_of_rows; ++i) {
        for (size_t j = 0; j < number_of_columns; ++j) {
            int len = snprintf(buffer, sizeof(buffer), "%8.2f", matrix[i][j]);
            if (len > 0) {
                write(STDOUT_FILENO, buffer, (size_t)len);
            }
            write(STDOUT_FILENO, &space, 1);
        }
        write(STDOUT_FILENO, &newline, 1);
    }
}
