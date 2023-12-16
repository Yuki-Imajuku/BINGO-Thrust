#ifndef NUMBER_GENERATOR_H
#define NUMBER_GENERATOR_H

#include <thrust/device_vector.h>

class NumberGenerator {
public:
    thrust::device_vector<char> generateNumbers(unsigned long seed);
};

#endif
