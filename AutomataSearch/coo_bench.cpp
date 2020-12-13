#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include <benchmark/benchmark.h>

#define RUNTIME_ENABLE_JIT
#include <anydsl_runtime.hpp>

const std::string ARTIC_SOURCE = "automata.impala";

struct CooDfaElement 
{
    int32_t from, to;
    char trans_symbol;
};

std::string read_string(const std::string &path)
{
    std::ios::sync_with_stdio(false);

    std::ifstream in(path);
    std::string result;
    getline(in, result);
    in.close();

    return result;
}

std::vector<CooDfaElement> read_coo(const std::string &path)
{
    std::ios::sync_with_stdio(false);
    std::ifstream in(path);

    std::vector<CooDfaElement> result;
    CooDfaElement current;
    while (in >> current.from >> current.trans_symbol >> current.to)
    {
        result.push_back(current);
    }

    in.close();
    return result;
}

std::string emit_coo_as_function(const std::vector<CooDfaElement> &coo)
{
    std::string result = "fn @second(i : i32) -> CooAutomataElement {\n\
                          match i {\n";
    for (int i = 0; i < coo.size(); ++i)
    {
        result += std::to_string(i) + " => CooAutomataElement { " + 
                    "state: " + std::to_string(coo[i].from) + ", " +
                    "symbol: " + std::to_string((uint8_t)coo[i].trans_symbol) + " as u8, " +
                    "next_state: " + std::to_string(coo[i].to) + "},\n";
    }
    result += "_ => CooAutomataElement { state: 0, symbol: 0 as u8, next_state: 0}\n";
    result += "} \n }\n";

    //std::cout << result << std::endl;

    return result;
}

std::string read_source()
{
    std::ifstream source_file(ARTIC_SOURCE);
    std::string result = "";

    std::string current_line;
    while (std::getline(source_file, current_line)) 
    {
        result += current_line + '\n';
    }

    return result;
}

typedef int32_t(*exec_fn)(const char *, int32_t);
exec_fn compile_spec_function(const std::vector<CooDfaElement> &coo)
{
    std::string program_string = 
        read_source() +
        emit_coo_as_function(coo) +

"extern \
fn wrapper(source : &[u8], source_length: i32) -> i32 \n\
{\n\
    regex_sparse_coo(\
            source, source_length, second, " + 
            std::to_string(coo.size()) + ", " + 
            std::to_string(std::max_element(coo.begin(), coo.end(), [](auto a, auto b) { return a.to < b.to;  })->to) + ")\n }\n"
;

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


static void BM_pdf_email_coo(benchmark::State &state)
{
    auto left = read_string("Data/Source/1.in");
    auto right = read_coo("Data/Pattern/Coo/email");

//    std::cout << left.length() << std::endl;
//    std::cout << right.size() << std::endl;
    
    auto wrapper = compile_spec_function(right);

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    }
}
BENCHMARK(BM_pdf_email_coo);

static void BM_pdf_email_strict_coo(benchmark::State &state)
{
    auto left = read_string("Data/Source/1.in");
    auto right = read_coo("Data/Pattern/Coo/email-strict");
    
//    std::cout << left.length() << std::endl;
//    std::cout << right.size() << std::endl;

    auto wrapper = compile_spec_function(right);

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    }
}
BENCHMARK(BM_pdf_email_strict_coo);


static void BM_pdf_credit_card_coo(benchmark::State &state)
{
    auto left = read_string("Data/Source/1.in");
    auto right = read_coo("Data/Pattern/Coo/card");
    
    auto wrapper = compile_spec_function(right);

    for (auto _ : state)
    {
        wrapper(left.c_str(), left.size());
    }
}
BENCHMARK(BM_pdf_credit_card_coo);

BENCHMARK_MAIN();

