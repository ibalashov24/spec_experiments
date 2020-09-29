#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#define RUNTIME_ENABLE_JIT
#include <anydsl_runtime.h>

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
    std::string result = "fn @second(i : i32) -> CooElement { \
                          match i {\
                              ";
    for (int i = 0; i < matrix_size; ++i)
    {
        result += std::to_string(i) + " => " + std::to_string(matrix[i].value) + ",";
    }

    result += "}; }";

    return result;
}

typedef void(*exec_fn)(CooElement *, int32_t, CooElement *, CooElement *);

exec_fn compile_spec_function(
        const std::vector<CooElement> &matrix,
        int32_t matrix_size)
{
    std::string program_string =
" struct CooElement {\
       row : i32,\
       col : i32,\
       value : f64\
}" + generate_function_matrix(matrix, matrix_size) +

"fn @(?a & ?b & ?s) unroll_step(a : i32, @b : i32, @s : i32, f : fn(i32) -> ()) -> () {\
    if a < b { \
        @@f(a); \
        unroll_step(a + s, b, s, f) \
    }\
}\
fn @unroll(a : i32, b : i32, f : fn(i32) -> ()) -> () { \
    unroll_step(a, b, 1, f) \
}\
\
// Calculates the product of two sparse matrices\
// Both in COO format\
extern\
fn @mult_matrix_matrix(\
        first_matrix: &[CooElement],\
        first_matrix_nonzero: i32,\
        second_matrix: fn(i32) -> f64,\
        second_matrix_nonzero : i32,\
        buffer: &mut [CooElement],\
        result: &mut [CooElement]) -> () {\
\
    let mut current_buffer_pos : i32 = 0; \
    for i in unroll(0, first_matrix_nonzero) {\
        for j in unroll(0, second_matrix_nonzero) {\
            if second_matrix(j).row == first_matrix(i).col {\
                let new_value : f64 = second_matrix(j).value * first_matrix(i).value;\
                buffer(current_buffer_pos).value = new_value;\
\
        \
                buffer(current_buffer_pos).row = first_matrix(i).row;\
                buffer(current_buffer_pos).col = second_matrix(j).col;\
                current_buffer_pos += 1\
            }\
        }\
    }\
\
    let mut current_result_pos : i32 = 0;\
    for i in unroll(0, current_buffer_pos) {\
        for j in unroll(0, current_result_pos + 1) {\
            if j == current_result_pos {\
                result(current_result_pos) = buffer(i);\
                current_result_pos += 1;\
                break()\
            } else if result(j).row == buffer(i).row && result(j).col == buffer(i).col {\
                result(j).value += buffer(i).value\
            }\
        }\
    }\
\
    *result_size = current_result_pos \
}\
\
fn @wrapper( \
        first : $[CooElement], first_count : i32, \
        buffer: &mut [CooElement],\
        result: &mut [CooElement]) -> () \
{\
    mult_matrix_matrix(first, first_count, second, " + std::to_string(matrix_size)
    + "buffer, result, result_size);\
}\
"; 
    auto key = anydsl_compile(
            program_string.c_str(), 
            (uint32_t) program_string.size(),
            3);

    if (auto ptr = anydsl_lookup_function(key, "wrapper")) {
        auto fun = reinterpret_cast<exec_fn>(ptr);
    } else {
        std::cout << "Compilation failed!" << std::endl;
    }

    return nullptr;
}

static void BM_mult_bcsstk16_2blocks(benchmark::State &state)
{
    auto left_matrix = read_matrix("Matrix/bcsstk16"); // Size 147631
    auto right_matrix = read_matrix("Matrix/2blocks"); // Size 8

    auto buffer = new CooElement[2000000];
    auto result = new CooElement[2000000];
   
    auto wrapper = compile_spec_function(right_matrix, 8);

    for (auto _ : state)
    {
        wrapper(left_matrix.data(), 147631, buffer, result);
    } 

    delete [] buffer;
    delete [] result;
}
BENCHMARK(BM_mult_bcsstk16_2blocks);

/*static void BM_mult_fs1831_2blocks(benchmark::State &state)
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
BENCHMARK(BM_mult_bcsstk16_2blocks);*/


BENCHMARK_MAIN();

