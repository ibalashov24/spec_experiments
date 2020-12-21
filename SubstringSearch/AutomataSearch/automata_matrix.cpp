#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <numeric_limits>

const int32_t ALPHABET_SIZE = 255;

int32_t regex_dense(
        const char *string,
        int32_t string_length,
        const uint8_t *automata,
        int32_t state_count)
{
    for (int i = 0; i < string_length; ++i)
    {
        int32_t current_state = 1;
        int32_t j = 0;
        while (current_state != state_count)
        {
            auto next_state = automata[ALPHABET_SIZE * current_state + const_cast<int>(string[i + j] - 'a')]; 
            if (next_state == 0)
            {
                break;
            }

            current_state = next_state;
            ++j;
        }

        if (current_state == state_count)
        {
            return i;
        }
    }

    return -1;
}

struct CooAutomataElement {
    int32_t state, next_state;
    char symbol;
};

int32_t regex_sparse_coo(
        const char *string,
        int32_t string_length,
        const CooAutomataElement *automata,
        int32_t coo_size,
        int32_t state_count)
{
    for (int i = 0; i < string_length; ++i)
    {
        int32_t current_state = 1;
        while (current_state != state_count)
        { 
            for (int j = 0; j < coo_size + 1; ++j)
            {
                if (j == coo_size) 
                {
                    break;
                }

                if (automata[j].state == current_state && automata[j].symbol == string[i + j])
                {
                    current_state = automata[j].next_state;
                }

                if (current_state == state_count)
                {
                    return i;
                }
            }
        }
    }

    return -1;
}

int32_t regex_sparse_csr(
        const char *string,
        int32_t string_length,
        int32_t *state_start_pos,
        char *symbols,
        int32_t *next_state,
        int32_t csr_size,
        int32_t state_count)
{
    for (int i = 0; i < string_length; ++i)
    {
        int32_t current_state = 1;
        int32_t j = 0;
        while  (current_state != state_count)
        {
            bool is_found_next = false;
            for (int32_t k = state_start_pos[current_state]; k < state_start_pos[current_state + 1]; ++k)
            {
                if (symbols[k] == string[i + j])
                {
                    current_state = next_state[k];
                    is_found_next = true;

                    break;
                }
            }

            if (!is_found_next)
            {
                break;
            }
            ++j;
        }

        if (current_state == state_count) 
        {
            return i;    
        }
    }

    return -1;
}

int main()
{

}
