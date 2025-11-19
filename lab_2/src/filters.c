#include "filters.h"
#include <string.h>

double erosion_filter(double **matrix, size_t row, size_t column,
                      size_t number_of_rows, size_t number_of_columns) {
    double min = matrix[row][column];
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int new_row = (int)row + i;
            int new_column = (int)column + j;

            if (new_row >= 0 && new_row < (int)number_of_rows &&
                new_column >= 0 && new_column < (int)number_of_columns) {
                if (matrix[new_row][new_column] < min) {
                    min = matrix[new_row][new_column];
                }
            }
        }
    }
    return min;
}

double dilation_filter(double **matrix, size_t row, size_t column,
                       size_t number_of_rows, size_t number_of_columns) {
    double max = matrix[row][column];
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int new_row = (int)row + i;
            int new_column = (int)column + j;

            if (new_row >= 0 && new_row < (int)number_of_rows &&
                new_column >= 0 && new_column < (int)number_of_columns) {
                if (matrix[new_row][new_column] > max) {
                    max = matrix[new_row][new_column];
                }
            }
        }
    }
    return max;
}

typedef struct {
    const char *name;
    filter_function_t function;
    const char *description;
} filter_table_t;

filter_table_t available_filters[] = {
    {"erosion", erosion_filter, "Erosion (minimum in 3x3 neighborhood)"},
    {"dilation", dilation_filter, "Dilation (maximum in 3x3 neighborhood)"},
    {NULL, NULL, NULL}
};
