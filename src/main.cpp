#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <tuple>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include "bingo_card_processor.h"
#include "number_generator.h"


// function to read command line arguments
std::tuple<unsigned long, unsigned long long, std::string, bool, bool> read_args(int argc, char* argv[]) {
    // default value
    unsigned long seed = 42;  // random seed
    unsigned long long n_cards = 100;  // number of cards
    std::string file_dir = "results";  // output file
    bool verbose = false;  // verbose mode

    // help message
    if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -s, --seed    random seed (default: 42)" << std::endl;
        std::cout << "  -n, --n_cards number of cards (default: 100)" << std::endl;
        std::cout << "  -o, --output  output directory (default: results)" << std::endl;
        std::cout << "  -v, --verbose verbose mode" << std::endl;
        std::cout << "  -h, --help    show this help message and exit" << std::endl;
        return std::make_tuple(seed, n_cards, file_dir, verbose, false);
    }

    // command line args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-s" || arg == "--seed") && i + 1 < argc) {
            seed = std::stoul(argv[++i]);
        }
        else if ((arg == "-n" || arg == "--n_cards") && i + 1 < argc) {
            n_cards = std::stoull(argv[++i]);
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            file_dir = argv[++i];
        }
        else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else {
            std::cerr << "Invalid option: " << arg << std::endl;
            return std::make_tuple(seed, n_cards, file_dir, verbose, false);
        }
    }

    return std::make_tuple(seed, n_cards, file_dir, verbose, true);
}


void create_dir(const std::string& dir) {
    if (!std::filesystem::exists(dir)) {
        auto result = std::filesystem::create_directory(dir);
        if (!result) {
            std::cerr << "Failed to create directory: " << dir << std::endl;
            std::exit(EXIT_FAILURE);
        }
        std::cout << "Created directory: " << dir << std::endl;
    } else {
        std::cout << "Directory already exists: " << dir << std::endl;
    }
}


// main function
int main(int argc, char* argv[]) {
    // read command line arguments
    auto [seed, n_cards, output_dir, verbose, success] = read_args(argc, argv);
    if (!success) {
        return EXIT_FAILURE;
    }

    // experiment setting
    std::cout << "=======================================" << std::endl;
    std::cout << "     random seed: " << seed << std::endl;
    std::cout << " number of cards: " << n_cards << std::endl;
    std::cout << "output directory: " << output_dir << std::endl;
    std::cout << "=======================================" << std::endl;

    // create output directory
    if(output_dir.back() == '/') output_dir.pop_back();
    create_dir(output_dir);

    // generate numbers
    auto start_time = std::chrono::system_clock::now();
    NumberGenerator generator;
    const auto numbers = generator.generateNumbers(seed);
    if(verbose) {
        const thrust::host_vector<char> host_numbers = numbers;
        std::cout << "[numbers]" << std::endl;
        int tmp_cnt = 0;
        for(int i = 0; i <numbers.size(); ++i) {
            std::cout << std::format(" {0:2}", (int)host_numbers[i]);
            if(++tmp_cnt % 15 == 0) {
                std::cout << std::endl;
            }
        }
    }

    // simulate bingo using GPU
    unsigned long long batch_size = 1024 * 1024 * 128;  // About 4GB VRAM is consumed
    unsigned long long n_batches = (n_cards + batch_size - 1) / batch_size;
    thrust::host_vector<BingoResult> results;

    // batch processing
    for (unsigned long long i = 0; i < n_batches; ++i) {
        unsigned long long start = i * batch_size;
        unsigned long long end = std::min((i + 1) * batch_size, n_cards);
        BingoCardProcessor processor;
        thrust::host_vector<BingoResult> batch_results = processor.processCards(numbers, start, end, seed);
        results.insert(results.end(), batch_results.begin(), batch_results.end());
    }

    // check the number of results
    assert(results.size() == n_cards);
    auto end_time = std::chrono::system_clock::now();
    auto duration_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Elapsed time: " << duration_msec << " [msec]" << std::endl;

    // write results to file
    std::ofstream ofs1(output_dir + "/counts.bin", std::ios::binary);
    std::ofstream ofs2(output_dir + "/boards.bin", std::ios::binary);
    for(const auto& result : results) {
        // bingo_at (1 Byte)
        const char* bingo_at_char = reinterpret_cast<const char*>(&result.bingo_at);
        ofs1.write(bingo_at_char, sizeof(result.bingo_at));
        // card_hash_2 (5 Bytes)
        char card_hash_2_char[5];
        for(int i = 0; i < 5; ++i) {  // get 5 Bytes
            card_hash_2_char[4-i] = (result.card_hash_2 >> (i << 3)) & 0xFF;  // last 8 bits
        }
        ofs2.write(card_hash_2_char, sizeof(card_hash_2_char));
        // card_hash_1 (8 Bytes)
        char card_hash_1_char[8];
        for(int i = 0; i < 8; ++i) {  // get 8 Bytes
            card_hash_1_char[7-i] = (result.card_hash_1 >> (i << 3)) & 0xFF;  // last 8 bits
        }
        ofs2.write(card_hash_1_char, sizeof(card_hash_1_char));
    }
    ofs1.close();
    ofs2.close();

    // print results (for debug)
    // if(verbose) {
    //     std::cout << "[results]" << std::endl;
    //     for(const auto& result : results) {
    //         std::cout << std::format("{0:2} {1:#X} {2:#X}\n", (int)result.bingo_at, result.card_hash_2, result.card_hash_1);
    //     }
    // }

    // print output files (for debug)
    // if(verbose) {
    //     std::ifstream ifs1(output_dir + "/counts.bin", std::ios::binary);
    //     std::ifstream ifs2(output_dir + "/boards.bin", std::ios::binary);
    //     for(int i = 0; i < n_cards; ++i) {
    //         char bingo_at;
    //         ifs1.read(reinterpret_cast<char*>(&bingo_at), sizeof(bingo_at));
    //         char board[26], byte;
    //         for(int j = 0; j < 26; j += 2) {
    //             ifs2.read(reinterpret_cast<char*>(&byte), sizeof(byte));
    //             board[24-j] = byte & 0x0F;
    //             board[25-j] = (byte >> 4) & 0x0F;
    //         }
    //         for(int j = 0; j < 25; ++j) {
    //             std::cout << std::format("{0:2} ", (int)board[j+1] + j / 5 * 15);
    //             if(j % 5 == 4) {
    //                 std::cout << std::endl;
    //             }
    //         }
    //     }
    // }
	return EXIT_SUCCESS;
}
