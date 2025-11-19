#pragma once

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef double (*filter_function_t)(double**, size_t, size_t, size_t, size_t);

double erosion_filter(double **matrix, size_t row, size_t column,
                      size_t number_of_rows, size_t number_of_columns);
double dilation_filter(double **matrix, size_t row, size_t column,
                       size_t number_of_rows, size_t number_of_columns);
