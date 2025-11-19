#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

double** create_matrix(size_t number_of_rows, size_t number_of_columns);
void free_matrix(double **matrix, size_t number_of_rows);
void initialize_matrix(double **matrix, size_t number_of_rows, size_t number_of_columns);
void copy_matrix(double **source, double **destination, size_t number_of_rows, size_t number_of_columns);
void print_matrix(double **matrix, size_t number_of_rows, size_t number_of_columns);
