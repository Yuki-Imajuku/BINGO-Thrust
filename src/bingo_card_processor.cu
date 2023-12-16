#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>
#include <thrust/random.h>
#include <thrust/sequence.h>
#include <thrust/shuffle.h>
#include <thrust/transform.h>

#include "bingo_card_processor.h"

struct GenerateAndMark {
    unsigned long seed;
    const char* orig_card;

    GenerateAndMark(unsigned long _seed, const char* _card) : seed(_seed), orig_card(_card) {}

    __device__ BingoResult operator()(const int trial_idx) {
        thrust::default_random_engine rng(seed + trial_idx);
        char numbers[75], card[25];
        thrust::sequence(thrust::device, numbers, numbers + 75);  // 0, 1, 2, ..., 74
        thrust::shuffle(thrust::device, numbers, numbers + 75, rng);
        for(int i = 0; i < 25; i++) {
            card[i] = orig_card[i];  // copy (we can't use copy() because it's not a __host__ function)
        }

        BingoResult result{
            -1,  // not bingo
        };

        // check bingo
        char all_1 = -1;  // 0b11111111
        card[12] ^= all_1;  // free space
        for(char cnt = 0; cnt < 75; ++cnt) {
            // mark number
            char num = numbers[cnt], row_start = numbers[cnt] / 15 * 5;
            for(int i = row_start; i < row_start + 5; i++) {
                if(card[i] == num) {
                    card[i] ^= all_1;  // mark by bit-flip (turns N to -N-1)
                    break;
                }
            }

            // check row
            for(int row_start = 0; row_start < 25; row_start += 5) {
                bool bingo = true;
                for(int i = 0; i < 5; i++) {
                    int idx = row_start + i;
                    if(card[idx] >= 0) {
                        bingo = false;
                        break;  // not marked
                    }
                }
                if(bingo) {
                    result.bingo_at = cnt + 1;  // cnt starts from 0
                    return result;
                }
            }

            // check column
            for(int i = 0; i < 5; i++) {
                bool bingo = true;
                for(int row_start = 0; row_start < 25; row_start += 5) {
                    int idx = row_start + i;
                    if(card[idx] >= 0) {
                        bingo = false;
                        break;  // not marked
                    }
                }
                if(bingo) {
                    result.bingo_at = cnt + 1;  // cnt starts from 0
                    return result;
                }
            }

            // check diagonal
            bool bingo = true;
            for(int i = 0; i < 30; i += 6) {
                if(card[i] >= 0) {
                    bingo = false;
                    break;  // not marked
                }
            }
            if(bingo) {
                result.bingo_at = cnt + 1;  // cnt starts from 0
                return result;
            }
            for(int i = 4; i < 24; i += 4) {
                if(card[i] >= 0) {
                    bingo = false;
                    break;  // not marked
                }
            }
            if(bingo) {
                result.bingo_at = cnt + 1;  // cnt starts from 0
                return result;
            }
        }

        return result;
    }
};

thrust::host_vector<BingoResult> BingoCardProcessor::processCards(const thrust::device_vector<char>& card, unsigned long long start, unsigned long long end, unsigned long seed) {
    thrust::device_vector<BingoResult> results(end - start);

    thrust::transform(
        thrust::counting_iterator<int>(start),
        thrust::counting_iterator<int>(end),
        results.begin(),
        GenerateAndMark(seed, thrust::raw_pointer_cast(card.data()))
    );

    thrust::host_vector<BingoResult> host_results(results);
    return host_results;
}
