#include <stdio.h>
#include <stdint.h>

struct CooElement {
	uint32_t row, col;
	double value;
};

void mult_matrix_vector(
		struct CooElement *coo, 
		int32_t matrix_rows,
		int32_t matrix_cols,
		int32_t nonzero_count,
		double *vector,
		int32_t vector_size,
		double *result); 

int main()
{
	struct CooElement coo[3];
	coo[0] = (struct CooElement) { 0, 0, 10.0 };
	coo[1] = (struct CooElement) { 0, 1, 1.0 };
	coo[2] = (struct CooElement) { 1, 1, 2.0 };


//	printf("%d %d %f", coo[1].row, coo[1].col, coo[1].value);

	double vector[2];
	vector[0] = -1.0;
	vector[1] = 0.5;

	double result[2];
	
	mult_matrix_vector(coo, 2, 2, 3, vector, 2, result);	

	for (int i = 0; i < 2; ++i)
		printf("%f\n", result[i]);

	return 0;
}
