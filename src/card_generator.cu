#include <thrust/copy.h>
#include <thrust/device_vector.h>
#include <thrust/random.h>
#include <thrust/sequence.h>
#include <thrust/shuffle.h>

#include "card_generator.h"

thrust::device_vector<char> CardGenerator::generateCard(unsigned long seed) {
    thrust::default_random_engine rng(seed);  // random number generator
    thrust::device_vector<char> numbers(25);  // vector to represent card
    for(char row_start = 0; row_start < 25; row_start += 5) {
        thrust::device_vector<char> row(15);
        thrust::sequence(row.begin(), row.end(), row_start * 3);
        thrust::shuffle(row.begin(), row.end(), rng);
        thrust::copy(row.begin(), row.begin() + 5, numbers.begin() + row_start);
    }
    return numbers;
}
