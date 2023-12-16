#ifndef NUMBER_GENERATOR_H
#define NUMBER_GENERATOR_H

#include <thrust/device_vector.h>

class CardGenerator {
public:
    thrust::device_vector<char> generateCard(unsigned long seed);
};

#endif
