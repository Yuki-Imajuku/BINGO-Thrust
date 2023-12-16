# BINGO-Thrust

BINGO simulation by generating many orders of numbers called randomly. Only 1 BINGO card is used, and all orders of numbers called is used to open that card.

Please see another version: [another branch](https://github.com/Yuki-Imajuku/BINGO-Thrust/tree/1-number-many-cards)

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
number of trials: 124511410
output directory: results
=======================================
Directory already exists: results
[card]
  4  1 14 13 10
 28 20 22 25 24
 35 40 31 37 42
 49 53 57 55 52
 62 70 71 72 69
Elapsed time: 27279 [msec]

# Analyze
make analyze

# Clear
make clear
```


## Results
Please see results directory.

- `card_seed[seed].txt`: BINGO card.
- `count_seed[seed].txt`: frequency for each timing when bingo is completed.
- `histogram_seed[seed].png`: histogram of the timing when bingo is completed.
