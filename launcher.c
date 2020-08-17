#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

struct CooElement {
	int32_t row, col;
	double value;
};

struct CooVectorElement {
    int32_t pos;
    double value;
};

void mult_matrix_vector(
		struct CooElement *coo, 
		int32_t matrix_rows,
		int32_t matrix_cols,
		int32_t nonzero_count,
		const struct CooVectorElement *vector,
		int32_t vector_nonzero_count,
        struct CooVectorElement *buffer,
		struct CooVectorElement *result,
        int32_t *result_size); 


void println(int32_t val)
{
    printf("%" PRId32 "\n", val);
}

int main()
{
	struct CooElement coo[3];
	coo[0] = (struct CooElement) { 0, 0, 10.0 };
	coo[1] = (struct CooElement) { 0, 1, 1.0 };
	coo[2] = (struct CooElement) { 1, 1, 2.0 };

    struct CooVectorElement vector[1];
    vector[0] = (struct CooVectorElement) { 1, 5.0 };

    struct CooVectorElement buffer[8];	
    struct CooVectorElement result[8];
    int32_t result_size;
	mult_matrix_vector(coo, 2, 2, 3, vector, 1, buffer, result, &result_size);	

    printf("Result size: %" PRId32 "\n", result_size);
    printf("Result vector: \n");
	for (int i = 0; i < result_size; ++i)
		printf("%d %f\n", result[i].pos, result[i].value);

	return 0;
}
