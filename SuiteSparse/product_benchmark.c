extern "C" {
#include <GraphBLAS.h>
}
#include <benchmark/benchmark.h>

#include <algorithm>
#include <fstream>

struct CooElement 
{
    int from, to;
    double value;
};

GrB_Matrix read_matrix(const std::string &file_name)
{
    std::ifstream input(file_name);
    
    std::vector<CooElement> coo_elements;
    CooElement current;
    int width = 0;
    int height = 0;
    while (input >> current.from >> current.to >> current.value) 
    {
        width = std::max(width, current.from);
        height = std::max(height, current.to);
        
        coo_elements.push_back(current);
    }

    GrB_Matrix result;
    GrB_Matrix_new(&result, GrB_FP64, height, width); 
    for (auto e : coo_elements)
    {
        GrB_Matrix_setElement_FP64(result, e.value, e.from, e.to);
    }

    return result;
}

void fit_matrix(GrB_Matrix *left, GrB_Matrix *right)
{
    GrB_Index nrows;
    GrB_Matrix_nrows(&nrows, *left);

    GrB_Index ncols;
    GrB_Matrix_ncols(&ncols, *right);

    GxB_Matrix_resize(*left, nrows, ncols);
}


// BENCHMARKS

void BM_product_bcsstk16_2blocks(benchmark::State& state)
{
    GrB_init(GrB_BLOCKING);

    GrB_Matrix left = read_matrix("Matrix/bcsstk16");
    GrB_Matrix right = read_matrix("Matrix/2blocks");

    fit_matrix(&left, &right);

    for (auto _ : state)
        GrB_mxm(left, GrB_NULL, GrB_NULL, GxB_PLUS_TIMES_FP64, left, right, GrB_NULL);

    GrB_Matrix_free(&left);
    GrB_Matrix_free(&right);

    GrB_finalize();
}
BENCHMARK(BM_product_bcsstk16_2blocks);


void BM_product_bcsstk16_eye3(benchmark::State& state)
{
    GrB_init(GrB_BLOCKING);

    GrB_Matrix left = read_matrix("Matrix/bcsstk16");
    GrB_Matrix right = read_matrix("Matrix/eye3");

    fit_matrix(&left, &right);

    for (auto _ : state)
        GrB_mxm(left, GrB_NULL, GrB_NULL, GxB_PLUS_TIMES_FP64, left, right, GrB_NULL);

    GrB_Matrix_free(&left);
    GrB_Matrix_free(&right);

    GrB_finalize();
}
BENCHMARK(BM_product_bcsstk16_eye3);

void BM_product_fs1831_eye3(benchmark::State& state)
{
    GrB_init(GrB_BLOCKING);

    GrB_Matrix left = read_matrix("Matrix/fs_183_1");
    GrB_Matrix right = read_matrix("Matrix/eye3");

    fit_matrix(&left, &right);

    for (auto _ : state)
        GrB_mxm(left, GrB_NULL, GrB_NULL, GxB_PLUS_TIMES_FP64, left, right, GrB_NULL);

    GrB_Matrix_free(&left);
    GrB_Matrix_free(&right);

    GrB_finalize();
}
BENCHMARK(BM_product_fs1831_eye3);

void BM_product_fs1831_2block(benchmark::State& state)
{
    GrB_init(GrB_BLOCKING);

    GrB_Matrix left = read_matrix("Matrix/fs_183_1");
    GrB_Matrix right = read_matrix("Matrix/2blocks");

    fit_matrix(&left, &right);

    for (auto _ : state)
        GrB_mxm(left, GrB_NULL, GrB_NULL, GxB_PLUS_TIMES_FP64, left, right, GrB_NULL);

    GrB_Matrix_free(&left);
    GrB_Matrix_free(&right);

    GrB_finalize();
}
BENCHMARK(BM_product_fs1831_2block);


BENCHMARK_MAIN();
