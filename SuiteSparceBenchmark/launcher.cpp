#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

#include <benchmark/benchmark.h>
#include <GraphBLAS.h>

struct CooElement {
	int32_t row, col;
	double value;
};

GrB_Matrix read_matrix(const std::string &file_name)
{
    std::ifstream input_matrix(file_name);

    std::vector<GrB_Index> row, col;
    std::vector<double> value;

    double current_value;
    uint32_t current_row, current_col;
    uint32_t col_number = 0, row_number = 0;
    while (input_matrix >> current_row >> current_col >> current_value) {
        row.push_back(current_row);
        col.push_back(current_col);
        value.push_back(current_value);

        row_number = std::max(row_number, current_row + 1);
        col_number = std::max(col_number, current_col + 1);
    }

    input_matrix.close();

    GrB_Matrix matrix;
    GrB_Matrix_new (&matrix, (GrB_Type) GrB_FP64, 4, 4); 
    GrB_Matrix_build_FP64( 
            matrix, 
            row.data(), col.data(), value.data(),
            row.size(), 
            GrB_SECOND_FP64);

    return matrix;
}

static void BM_mult_bcsstk16_2blocks(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/bcsstk16"); // Size 147631
    auto right_matrix = read_matrix("Matrix/2blocks"); // Size 8

    for (auto _ : state)
    {
        GrB_mxm
        (
            left_matrix, 
            right_matrix,
            NULL,
            GxB_PLUS_TIMES_UINT32,
            left_matrix,
            right_matrix,
            NULL);
    } 

    GrB_Matrix_free(&left_matrix);
    GrB_Matrix_free(&right_matrix);
}
BENCHMARK(BM_mult_bcsstk16_2blocks);

static void BM_mult_fs1831_2blocks(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/fs_183_1"); // Size 1069 
    auto right_matrix = read_matrix("Matrix/2blocks"); // Size 8

    for (auto _ : state)
    {
        GrB_mxm
        (
            left_matrix, 
            right_matrix,
            NULL,
            GxB_PLUS_TIMES_UINT32,
            left_matrix,
            right_matrix,
            NULL);
    } 

    GrB_Matrix_free(&left_matrix);
    GrB_Matrix_free(&right_matrix);
}
BENCHMARK(BM_mult_fs1831_2blocks);

static void BM_mult_bccsstk16_eye3(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/bcsstk16"); // Size 147631
    auto right_matrix = read_matrix("Matrix/eye3"); // Size 3

    for (auto _ : state)
    {
        GrB_mxm
        (
            left_matrix, 
            right_matrix,
            NULL,
            GxB_PLUS_TIMES_UINT32,
            left_matrix,
            right_matrix,
            NULL);
    } 

    GrB_Matrix_free(&left_matrix);
    GrB_Matrix_free(&right_matrix);
}
BENCHMARK(BM_mult_bccsstk16_eye3);

static void BM_mult_fs1832_eye3(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/fs_183_1"); // Size 1069
    auto right_matrix = read_matrix("Matrix/eye3"); // Size 3

    for (auto _ : state)
    {
        GrB_mxm
        (
            left_matrix, 
            right_matrix,
            NULL,
            GxB_PLUS_TIMES_UINT32,
            left_matrix,
            right_matrix,
            NULL);
    } 

    GrB_Matrix_free(&left_matrix);
    GrB_Matrix_free(&right_matrix);
}
BENCHMARK(BM_mult_fs1832_eye3);

BENCHMARK_MAIN();

