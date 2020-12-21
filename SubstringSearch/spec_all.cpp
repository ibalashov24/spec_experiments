#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include <benchmark/benchmark.h>

#define RUNTIME_ENABLE_JIT
#include <anydsl_runtime.hpp>

std::string read_string(const std::string &path)
{
    std::ios::sync_with_stdio(false);

    std::ifstream in(path);
    std::string result;
    getline(in, result);
    in.close();

    return result;
}

std::string generate_function_string(const std::string &str)
{
    std::string result = "fn @second(i : i32) -> u8 {\n\
                          match i {\n";
    for (int i = 0; i < str.size(); ++i)
    {
        result += std::to_string(i) + " => " + std::to_string((uint8_t)str[i]) + " as u8,\n";
    }
    result += "_ => 0 as u8\n";
    result += "} }\n";

    //std::cout << result << std::endl;

    return result;
}

typedef int32_t(*exec_fn)(const char *, int32_t);

exec_fn compile_spec_function(const std::string &right)
{
    std::string program_string = generate_function_string(right) +
"\n\
extern\n\
fn @tensor_product(\n\
        source: &[u8],\n\
        source_length: i32,\n\
        pattern: fn(i32) -> u8,\n\
        pattern_length: i32) -> i32 {\
\n\
    for i in unroll(0, source_length) {\n\
        for j in unroll(0, pattern_length + 1) {\n\
            if j == pattern_length { \n\
                return(i)\n\
            }\
            \
            if source(i + j) != pattern(j) {\n\
                break()\n\
            }\n\
        }\n\
    }\n\
\
    return(-1)\
}\n\
\n\
extern \
fn wrapper( \n\
        first : &[u8], first_length: i32) -> i32\
{\n\
    tensor_product(first, first_length, second, " + 
        std::to_string(right.size()) + ")\n\
}\n\
";

//std::cout << program_string << std::endl;

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

static void BM_substr_20000000_1(benchmark::State &state)
{
    auto left = read_string("Strings/Source/1.in");
    auto right = read_string("Strings/Pattern/1.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_20000000_1);


static void BM_substr_20000000_2(benchmark::State &state)
{
    auto left = read_string("Strings/Source/2.in");
    auto right = read_string("Strings/Pattern/1.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_20000000_2);


static void BM_substr_20000000_3(benchmark::State &state)
{
    auto left = read_string("Strings/Source/1.in");
    auto right = read_string("Strings/Pattern/2.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_20000000_3);


static void BM_substr_20000000_4(benchmark::State &state)
{
    auto left = read_string("Strings/Source/2.in");
    auto right = read_string("Strings/Pattern/2.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_20000000_4);

static void BM_substr_exact(benchmark::State &state)
{
    auto left = read_string("Strings/Source/3.in");
    auto right = read_string("Strings/Pattern/2.in"); 

    std::cout << left.size() << std::endl;

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_exact);

static void BM_substr_exact_short(benchmark::State &state)
{
    auto left = read_string("Strings/Source/0.in");
    auto right = read_string("Strings/Pattern/0.in"); 

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    } 
}
BENCHMARK(BM_substr_exact_short);

static void BM_huge_source_1(benchmark::State &state)
{
    std::ifstream in("Strings/Source/pdfsdump");
    std::stringstream buffer;
    buffer << in.rdbuf();

    auto right = read_string("Strings/Pattern/snort3-community.rules");

    std::cout << buffer.str().size() << std::endl;

    auto wrapper = compile_spec_function(right.c_str());

    for (auto _ : state)
    {
        wrapper(buffer.str().c_str(), buffer.str().size());
    } 
}
BENCHMARK(BM_huge_source_1);

BENCHMARK_MAIN();

