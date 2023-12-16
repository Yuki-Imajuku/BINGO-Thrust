#ifndef BINGO_CARD_PROCESSOR_H
#define BINGO_CARD_PROCESSOR_H

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

struct BingoResult {
    char bingo_at;
};

class BingoCardProcessor {
public:
    thrust::host_vector<BingoResult> processCards(const thrust::device_vector<char>& card, unsigned long long start, unsigned long long end, unsigned long seed);
};

#endif
