#ifndef BINGO_CARD_PROCESSOR_H
#define BINGO_CARD_PROCESSOR_H

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

struct BingoResult {
    char bingo_at;
    unsigned long long card_hash_1, card_hash_2;
};

class BingoCardProcessor {
public:
    thrust::host_vector<BingoResult> processCards(const thrust::device_vector<char>& numbers, unsigned long long start, unsigned long long end, unsigned long seed);
};

#endif
