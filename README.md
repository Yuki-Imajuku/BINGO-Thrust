# BINGO-Thrust

BINGO simulation by generating many BINGO cards randomly. All cards are opened in one fixed order of numbers called.

Please see another version: [another branch](https://github.com/Yuki-Imajuku/BINGO-Thrust/tree/many-numbers-1-card)

This project uses `make`, `nvcc (CUDA 12.1, std=c++20)`, `Python 3.11`.
Tested on RTX 3070 (Windows 11).

```sh
# Setup
make init

# Build
make

# Execute
./bin/bingo -v -n 124511410 -s 40
=======================================
     random seed: 40
 number of cards: 124511410
output directory: results
=======================================
Directory already exists: results
[numbers]
  9 55 58 24  5  4 14 68 71 73 49 51 15 18 72
 46 45 25 37 48 60 13 41 28 23 61 20 64 11  7
 47 52 26 39 74 65 27 43 66 59 29  3 36 69 32
  1 19 21 56 42 50 30  8 38 31 57 33  6 16 67
 54 40 22 63 34 12  0 53 17 62 70  2 10 35 44
Elapsed time: 103941 [msec]

# Analyze
make analyze

# Clear
make clear
```


## Results
Please see [results](./results/) directory.

- `numbers_seed[seed].txt`: order of numbers called.
- `count_seed[seed].txt`: frequency for each timing when bingo is completed.
- `histogram_seed[seed].png`: histogram of the timing when bingo is completed. 
