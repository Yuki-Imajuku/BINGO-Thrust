#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>
#include <thrust/random.h>
#include <thrust/sequence.h>
#include <thrust/shuffle.h>
#include <thrust/transform.h>

#include "bingo_card_processor.h"

struct GenerateAndMark {
    unsigned long seed;
    const char* numbers;

    GenerateAndMark(unsigned long _seed, const char* _numbers) : seed(_seed), numbers(_numbers) {}

    __device__ BingoResult operator()(const int card_idx) {
        unsigned long long row_hash = 0;
        thrust::default_random_engine rng(seed + card_idx);
        char card[25];

        BingoResult result{
            -1,  // not bingo
            0,  // Hash Result of `B`, `I`, `N` column (for simplification, column are treated as row)
            0,  // Hash Result of `G`, `O` column (for simplification, column are treated as row)
        };

        // mark card
        for(int row_start = 0; row_start < 25; row_start += 5) {
            char row[15];
            thrust::sequence(thrust::device, row, row + 15);  // 0, 1, 2, ..., 14
            thrust::shuffle(row, row + 15, rng);
            row_hash = 0;
            for(int i = 0; i < 5; i++) {
                int idx = row_start + i;
                card[idx] = row[i] + (row_start << 1) + row_start;  // add row_start * 3 (= row_idx * 15) to realize `B`, `I`, `N`, `G`, `O` column rule
                row_hash += (unsigned long long)row[i] << (i << 2);  // 4 bits per number (i << 2 == i * 4)
            }
            if(row_start < 15) {
                result.card_hash_1 += row_hash << (row_start << 2);  // 20 bits per row (row_start << 2 == row_start * 4)
            }
            else {
                result.card_hash_2 += row_hash << ((row_start - 15) << 2);  // 20 bits per row (row_start << 2 == row_start * 4)
            }
        }
        result.card_hash_1 <<= 4;  // 4 bit for free space

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

thrust::host_vector<BingoResult> BingoCardProcessor::processCards(const thrust::device_vector<char>& numbers, unsigned long long start, unsigned long long end, unsigned long seed) {
    thrust::device_vector<BingoResult> results(end - start);

    thrust::transform(
        thrust::counting_iterator<int>(start),
        thrust::counting_iterator<int>(end),
        results.begin(),
        GenerateAndMark(seed, thrust::raw_pointer_cast(numbers.data()))
    );

    thrust::host_vector<BingoResult> host_results(results);
    return host_results;
}
