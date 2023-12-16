#include <thrust/device_vector.h>
#include <thrust/random.h>
#include <thrust/sequence.h>
#include <thrust/shuffle.h>

#include "number_generator.h"

thrust::device_vector<char> NumberGenerator::generateNumbers(unsigned long seed) {
    thrust::default_random_engine rng(seed);  // random number generator
    thrust::device_vector<char> numbers(75);  // Bingo uses 75 numbers
    thrust::sequence(numbers.begin(), numbers.end());  // 0, 1, 2, ..., 74
    thrust::shuffle(numbers.begin(), numbers.end(), rng);  // shuffle numbers
    return numbers;
}
