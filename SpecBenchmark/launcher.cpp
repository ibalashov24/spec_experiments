#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

struct CooElement {
	int32_t row, col;
	double value;
};

extern "C"
void mult_matrix_matrix(
        CooElement *first_matrix,
        int32_t first_matrix_nonzero,
        CooElement *second_matrix,
        int32_t second_matrix_nonzero,
        CooElement *buffer,
        CooElement *result,
        int32_t *result_size);

std::vector<CooElement> read_matrix(const std::string &file_name)
{
    std::ifstream input_matrix(file_name);

    std::vector<CooElement> matrix;

    CooElement temp_element;
    while (input_matrix >> temp_element.row >> temp_element.col >> temp_element.value)
    {
       matrix.push_back(temp_element);
    }

    return matrix;
}

static void BM_mult_bcsstk16_2blocks(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/bcsstk16"); // Size 147631
    auto right_matrix = read_matrix("Matrix/2blocks"); // Size 8

    auto buffer = new CooElement[2000000];
    auto result = new CooElement[2000000];
    auto result_size = new int;
    
    for (auto _ : state)
    {

        mult_matrix_matrix(left_matrix.data(), 147631,
                           right_matrix.data(), 8,
                           buffer, result, result_size); 
    } 

    delete [] buffer;
    delete [] result;
    delete result_size;
}
BENCHMARK(BM_mult_bcsstk16_2blocks);

static void BM_mult_fs1831_2blocks(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/fs_183_1"); // Size 1069 
    auto right_matrix = read_matrix("Matrix/2blocks"); // Size 8

    auto buffer = new CooElement[10000];
    auto result = new CooElement[10000];
    auto result_size = new int;
    
    for (auto _ : state)
    {
        mult_matrix_matrix(left_matrix.data(), 1069,
                           right_matrix.data(), 8,
                           buffer, result, result_size); 
    } 

    delete [] buffer;
    delete [] result;
    delete result_size;
}
BENCHMARK(BM_mult_fs1831_2blocks);

static void BM_mult_bccsstk16_eye3(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/bcsstk16"); // Size 147631
    auto right_matrix = read_matrix("Matrix/eye3"); // Size 3

    auto buffer = new CooElement[2000000];
    auto result = new CooElement[2000000];
    auto result_size = new int;
    
    for (auto _ : state)
    {
        mult_matrix_matrix(left_matrix.data(), 147631,
                           right_matrix.data(), 3,
                           buffer, result, result_size); 
    } 

    delete [] buffer;
    delete [] result;
    delete result_size;
}
BENCHMARK(BM_mult_bccsstk16_eye3);

static void BM_mult_fs1832_eye3(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/fs_183_1"); // Size 1069
    auto right_matrix = read_matrix("Matrix/eye3"); // Size 3

    auto buffer = new CooElement[4000];
    auto result = new CooElement[4000];
    auto result_size = new int;
    
    for (auto _ : state)
    {
        mult_matrix_matrix(left_matrix.data(), 1069,
                           right_matrix.data(), 3,
                           buffer, result, result_size); 
    } 

    delete [] buffer;
    delete [] result;
    delete result_size;
}
BENCHMARK(BM_mult_bcsstk16_2blocks);


BENCHMARK_MAIN();

