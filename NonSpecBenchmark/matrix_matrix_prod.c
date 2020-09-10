#include <stdio.h>
#include <stdint.h>

struct CooElement {
    int32_t row, col;
    double value;
};

void mult_matrix_matrix(
        struct CooElement *first_matrix,
        int32_t first_matrix_nonzero,
        struct CooElement *second_matrix,
        int32_t second_matrix_nonzero,
        struct CooElement *buffer,
        struct CooElement *result,
        int32_t *result_size)
{
    int32_t current_buffer_pos = 0;
    for (size_t i = 0; i < first_matrix_nonzero; ++i)
        for (size_t j = 0; j < second_matrix_nonzero; ++j)
            if (second_matrix[j].row == first_matrix[i].col) {
                buffer[current_buffer_pos].value = 
                    second_matrix[j].value * first_matrix[i].value;

                buffer[current_buffer_pos].row = first_matrix[i].row;
                buffer[current_buffer_pos].col = second_matrix[j].col;
                ++current_buffer_pos;
            }   

    int32_t current_result_pos = 0;
    for (size_t i = 0; i < current_buffer_pos; ++i)
       for (size_t j = 0; j < current_result_pos + 1; ++j)
          if (j == current_result_pos) {
            result[current_result_pos] = buffer[i];
            ++current_result_pos;
            break;
          } else if (result[j].row == buffer[i].row &&
                     result[j].col == buffer[i].col) {
            result[j].value += buffer[i].value;
          }

    *result_size = current_buffer_pos;
}
