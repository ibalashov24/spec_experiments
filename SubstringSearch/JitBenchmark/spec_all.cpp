#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#define RUNTIME_ENABLE_JIT
#include <anydsl_runtime.hpp>

std::string read_string(const std::string &path)
{
    ios::sync_with_stdio(false);

    std::ifstream in(path);

    std::string result;
    in >> result;
    in.close();

    return result;
}

std::string generate_function_string(const std::string &str)
{
    std::string result = "fn @second(i : i32) -> u8 {\n\
                          match i {\n";
    for (int i = 0; i < str.size(); ++i)
    {
        result += std::to_string(i) + " => " + std::to_string((uint8_t)str[i]) + "\n";
    }
    result += "_ => 0" + \n";
    result += "} }\n";

    return result;
}

typedef void(*exec_fn)(uint8_t *, int32_t);

exec_fn compile_spec_function(const std::string &right)
{
    std::string program_string = generate_function_matrix(right) +
"\n\
extern\n\
fn @tensor_product(\n\
        source: &[u8],\n\
        source_length: i32,\n\
        pattern: fn(i32) -> CooElement,\n\
        pattern_length: i32) -> i32 {\
\n\
    for i in unroll(0, source_length) {\n\
        for j in unroll(0, pattern_length + 1) {\n\
            if (j == pattern_length) { \n\
                return i;\n\
            } \n\
            if source(i) != pattern(j) {\n\
                break();\n\
            }\n\
        }\n\
    }\n\
\
    return -1;\
}\n\
\n\
extern \
fn wrapper( \n\
        first : &[u8], first_length: i32) -> i32\
{\n\
    tensor_product(first, first_length, second, " + 
        std::to_string(right.size()) + ");\n\
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

static void BM_substr_200000_1(benchmark::State &state)
{
    auto right = read_matrix("Strings/Source/1.in");
    auto left = read_matrix("Strings/Pattern/1.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_200000_1);


static void BM_substr_20000_1(benchmark::State &state)
{
    auto right = read_matrix("Strings/Source/2.in");
    auto left = read_matrix("Strings/Pattern/1.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_20000_1);


static void BM_substr_200000_2(benchmark::State &state)
{
    auto right = read_matrix("Strings/Source/1.in");
    auto left = read_matrix("Strings/Pattern/2.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_200000_2);


static void BM_substr_20000_2(benchmark::State &state)
{
    auto right = read_matrix("Strings/Source/2.in");
    auto left = read_matrix("Strings/Pattern/2.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_20000_2);

static void BM_substr_exact(benchmark::State &state)
{
    auto right = read_matrix("Strings/Source/3.in");
    auto left = read_matrix("Strings/Pattern/2.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_exact);


BENCHMARK_MAIN();

