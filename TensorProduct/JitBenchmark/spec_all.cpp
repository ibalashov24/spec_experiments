#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#define RUNTIME_ENABLE_JIT
#include <anydsl_runtime.hpp>

#include "structures.hpp"

CooMatrix read_matrix(const std::string &file_name)
{
    std::ifstream input_matrix(file_name);

    CooMatrix result = { .coo=std::vector<CooElement>(), .row_count=0, .col_count=0 };

    CooElement temp_element;
    while (input_matrix >> temp_element.row >> temp_element.col >> temp_element.value)
    {
       result.coo.push_back(temp_element);

       result.row_count = std::max(result.row_count, temp_element.row);
       result.col_count = std::max(result.col_count, temp_element.col);
    }

    return result;
}

std::string generate_function_matrix(
        const std::vector<CooElement> &matrix)
{
    std::string result = "fn @second(i : i32) -> CooElement { \n\
                          match i {\n";
    for (int i = 0; i < matrix.size(); ++i)
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

typedef void(*exec_fn)(CooElement *, int32_t, int32_t, int32_t, CooElement *);

exec_fn compile_spec_function(const std::vector<CooElement> &matrix)
{
    std::string program_string = " \
 struct CooElement {\n\
       row : i32,\n\
       col : i32,\n\
       value : f64\n\
}\n" + generate_function_matrix(matrix) +

"\n\
// Calculates the product of two sparse matrices\n\
// Both in COO format\n\
extern\n\
fn @tensor_product(\n\
        first: &[CooElement],\n\
        first_rows: i32,\n\
        first_cols: i32,\n\
        first_nonzero: i32,\n\
        second_matrix: fn(i32) -> CooElement,\n\
        second_matrix_nonzero : i32,\n\
        result: &mut [CooElement]) -> () {\n\
\n\
    let mut current_result_pos : i32 = 0; \n\
    for i in unroll(0, first_nonzero) {\n\
        for j in unroll(0, second_matrix_nonzero) {\n\
            if second_matrix(j).row == first(i).col {\n\
                let new_value : f64 = second_matrix(j).value * first(i).value;\n\
                \n\
                result(current_result_pos).value = new_value;\n\
                result(current_result_pos).row = first(i).row * first_rows + second_matrix(j).row;\n\
                result(current_result_pos).col = first(i).col * first_cols + second_matrix(j).col;\n\
                current_result_pos += 1\n\
            }\n\
        }\n\
    }\n\
}\n\
\n\
extern \
fn wrapper( \n\
        first : &[CooElement], first_rows: i32, first_cols: i32, first_count : i32, \n\
        result: &mut [CooElement]) -> () \n\
{\n\
    tensor_product(first, first_rows, first_cols, first_count, second, " + 
        std::to_string(matrix.size()) + ", result);\n\
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
    auto right = read_matrix("Matrix/2blocks"); // Size 147631
    auto left = read_matrix("Matrix/bcsstk16"); // Size 8

    auto result = new CooElement[2000000];
    auto wrapper = compile_spec_function(right.coo);

    for (auto _ : state)
    {
        wrapper(left.coo.data(), left.row_count, left.col_count, 8, result);
    } 

    delete [] result;
}
BENCHMARK(BM_mult_bcsstk16_2blocks);

static void BM_mult_fs1831_2blocks(benchmark::State &state)
{
    auto left = read_matrix("Matrix/fs_183_1"); // Size 1069 
    auto right = read_matrix("Matrix/2blocks"); // Size 8

    auto result = new CooElement[10000];
    auto wrapper = compile_spec_function(right.coo);

    for (auto _ : state)
    {
        wrapper(left.coo.data(), left.row_count, left.col_count, 1069, result);
    }

    delete [] result;
}
BENCHMARK(BM_mult_fs1831_2blocks);

static void BM_mult_bccsstk16_eye3(benchmark::State &state)
{
    auto left = read_matrix("Matrix/bcsstk16"); // Size 147631
    auto right = read_matrix("Matrix/eye3"); // Size 3

    auto result = new CooElement[2000000];
    auto wrapper = compile_spec_function(right.coo);

    for (auto _ : state)
    {
        wrapper(left.coo.data(), left.row_count, left.col_count, 147631, result);
    }

    delete [] result;
}
BENCHMARK(BM_mult_bccsstk16_eye3);

static void BM_mult_fs1832_eye3(benchmark::State &state)
{
    auto left = read_matrix("Matrix/fs_183_1"); // Size 1069
    auto right = read_matrix("Matrix/eye3"); // Size 3
    
    auto result = new CooElement[4000];
    auto wrapper = compile_spec_function(right.coo);
    
    for (auto _ : state)
    {
        wrapper(left.coo.data(), left.row_count, left.col_count, 1069, result);
    } 

    delete [] result;
}
BENCHMARK(BM_mult_fs1832_eye3);


BENCHMARK_MAIN();

