#include <cstdint>
#include <vector>

struct CooElement {
	int32_t row, col;
	double value;
};

struct CooMatrix {
    std::vector<CooElement> coo;
    int32_t row_count, col_count;
};

