#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#define RUNTIME_ENABLE_JIT
#include <anydsl_runtime.hpp>

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

std::string generate_function_matrix(
        const std::vector<CooElement> &matrix,
        int32_t matrix_size)
{
    std::string result = "fn @second(i : i32) -> CooElement { \n\
                          match i {\n";
    for (int i = 0; i < matrix_size; ++i)
    {
        result += std::to_string(i) + " => CooElement {row : " + 
            std::to_string(matrix[i].row) + 
            ", col : " + std::to_string(matrix[i].col) +
            ", value : " + std::to_string(matrix[i].value) + "},\n";
    }
    result += "_ => CooElement {row : 0, col : 0, value : 1.000000}\n";
    result += "} }\n";

    return result;
}

typedef void(*exec_fn)(CooElement *, int32_t, CooElement *, CooElement *);

exec_fn compile_spec_function(
        const std::vector<CooElement> &matrix,
        int32_t matrix_size)
{
    std::string program_string = " \
 struct CooElement {\n\
       row : i32,\n\
       col : i32,\n\
       value : f64\n\
}\n" + generate_function_matrix(matrix, matrix_size) +

"\n\
// Calculates the product of two sparse matrices\n\
// Both in COO format\n\
extern\n\
fn @mult_matrix_matrix(\n\
        first_matrix: &[CooElement],\n\
        first_matrix_nonzero: i32,\n\
        second_matrix: fn(i32) -> CooElement,\n\
        second_matrix_nonzero : i32,\n\
        result: &mut [CooElement]) -> () {\n\
\n\
    let mut current_buffer_pos : i32 = 0; \n\
    for i in unroll(0, first_matrix_nonzero) {\n\
        for j in unroll(0, second_matrix_nonzero) {\n\
            if second_matrix(j).row == first_matrix(i).col {\n\
                let new_value : f64 = second_matrix(j).value * first_matrix(i).value;\n\
                for k in unroll(0, current_result_pos + 1) {\n\
                    if k == current_result_pos {\n\
                        result(current_result_pos) = new_value;\n\
                        current_result_pos += 1;\n\
                        break()\n\
                    } else if result(k).row == first_matrix(i).row && result(k).col == second_matrix(j).row {\n\
                        result(j).value += new_value\n\
                    }\n\
                }\n\
}\n\
\n\
extern \
fn wrapper( \n\
        first : &[CooElement], first_count : i32, \n\
        result: &mut [CooElement]) -> () \n\
{\n\
    mult_matrix_matrix(first, first_count, second, " + std::to_string(matrix_size)
    + ", result);\n\
}\n\
";
    auto key = anydsl_compile(
            program_string.c_str(), 
            (uint32_t) program_string.size(),
            2);

    if (auto ptr = anydsl_lookup_function(key, "wrapper")) {
        auto fun = reinterpret_cast<exec_fn>(ptr);
        return fun;
    } else {
        std::cout << "Compilation failed!" << std::endl;
    }

    return nullptr;
}

static void BM_mult_bcsstk16_2blocks(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/2blocks"); // Size 147631
    auto right_matrix = read_matrix("Matrix/bcsstk16"); // Size 8

    auto buffer = new CooElement[2000000];
    auto result = new CooElement[2000000];
   
    auto wrapper = compile_spec_function(right_matrix, 8);

    for (auto _ : state)
    {
        wrapper(left_matrix.data(), 147631, result);
    } 

    delete [] buffer;
    delete [] result;
}
BENCHMARK(BM_mult_bcsstk16_2blocks);

static void BM_mult_fs1831_2blocks(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/fs_183_1"); // Size 1069 
    auto right_matrix = read_matrix("Matrix/2blocks"); // Size 8

    auto buffer = new CooElement[10000];
    auto result = new CooElement[10000];
    
    auto wrapper = compile_spec_function(right_matrix, 8);

    for (auto _ : state)
    {
        wrapper(left_matrix.data(), 1069, result);
    }

    delete [] buffer;
    delete [] result;
}
BENCHMARK(BM_mult_fs1831_2blocks);

static void BM_mult_bccsstk16_eye3(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/bcsstk16"); // Size 147631
    auto right_matrix = read_matrix("Matrix/eye3"); // Size 3

    auto buffer = new CooElement[2000000];
    auto result = new CooElement[2000000];
 
    auto wrapper = compile_spec_function(right_matrix, 3);

    for (auto _ : state)
    {
        wrapper(left_matrix.data(), 147631, result);
    }

    delete [] buffer;
    delete [] result;
}
BENCHMARK(BM_mult_bccsstk16_eye3);

static void BM_mult_fs1832_eye3(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/fs_183_1"); // Size 1069
    auto right_matrix = read_matrix("Matrix/eye3"); // Size 3

    auto buffer = new CooElement[4000];
    auto result = new CooElement[4000];
    
    auto wrapper = compile_spec_function(right_matrix, 3);
    
    for (auto _ : state)
    {
        wrapper(left_matrix.data(), 1069, result);
    } 

    delete [] buffer;
    delete [] result;
}
BENCHMARK(BM_mult_fs1832_eye3);


BENCHMARK_MAIN();

