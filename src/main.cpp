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
#include "card_generator.h"


// function to read command line arguments
std::tuple<unsigned long, unsigned long long, std::string, bool, bool> read_args(int argc, char* argv[]) {
    // default value
    unsigned long seed = 42;  // random seed
    unsigned long long n_trials = 100;  // number of trials
    std::string file_dir = "results";  // output file
    bool verbose = false;  // verbose mode

    // help message
    if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -s, --seed    random seed (default: 42)" << std::endl;
        std::cout << "  -n, --n_trials number of trials (default: 100)" << std::endl;
        std::cout << "  -o, --output  output directory (default: results)" << std::endl;
        std::cout << "  -v, --verbose verbose mode" << std::endl;
        std::cout << "  -h, --help    show this help message and exit" << std::endl;
        return std::make_tuple(seed, n_trials, file_dir, verbose, false);
    }

    // command line args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-s" || arg == "--seed") && i + 1 < argc) {
            seed = std::stoul(argv[++i]);
        }
        else if ((arg == "-n" || arg == "--n_cards") && i + 1 < argc) {
            n_trials = std::stoull(argv[++i]);
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            file_dir = argv[++i];
        }
        else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else {
            std::cerr << "Invalid option: " << arg << std::endl;
            return std::make_tuple(seed, n_trials, file_dir, verbose, false);
        }
    }

    return std::make_tuple(seed, n_trials, file_dir, verbose, true);
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
    auto [seed, n_trials, output_dir, verbose, success] = read_args(argc, argv);
    if (!success) {
        return EXIT_FAILURE;
    }

    // experiment setting
    std::cout << "=======================================" << std::endl;
    std::cout << "     random seed: " << seed << std::endl;
    std::cout << "number of trials: " << n_trials << std::endl;
    std::cout << "output directory: " << output_dir << std::endl;
    std::cout << "=======================================" << std::endl;

    // create output directory
    if(output_dir.back() == '/') output_dir.pop_back();
    create_dir(output_dir);



    // generate numbers
    auto start_time = std::chrono::system_clock::now();
    CardGenerator generator;
    const auto card = generator.generateCard(seed);
    if(verbose) {
        const thrust::host_vector<char> host_card = card;
        std::cout << "[card]" << std::endl;
        for(int i = 0; i < host_card.size(); ++i) {
            std::cout << std::format(" {0:2}", (int)host_card[i]);
            if(i % 5 == 4) {
                std::cout << std::endl;
            }
        }
    }

    // set heap size to 2GB
    cudaError_t err = cudaDeviceSetLimit(cudaLimitMallocHeapSize, 1048576ULL*2048);  // shuffle uses heap memory ?

    // simulate bingo using GPU
    unsigned long long batch_size = 1024 * 1024 * 128;
    unsigned long long n_batches = (n_trials + batch_size - 1) / batch_size;
    thrust::host_vector<BingoResult> results;

    // batch processing
    for (unsigned long long i = 0; i < n_batches; ++i) {
        unsigned long long start = i * batch_size;
        unsigned long long end = std::min((i + 1) * batch_size, n_trials);
        BingoCardProcessor processor;
        thrust::host_vector<BingoResult> batch_results = processor.processCards(card, start, end, seed);
        results.insert(results.end(), batch_results.begin(), batch_results.end());
    }

    // check the number of results
    assert(results.size() == n_trials);
    auto end_time = std::chrono::system_clock::now();
    auto duration_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Elapsed time: " << duration_msec << " [msec]" << std::endl;

    // write results to file
    std::ofstream ofs(output_dir + "/counts.bin", std::ios::binary);
    for(const auto& result : results) {
        const char* bingo_at_char = reinterpret_cast<const char*>(&result.bingo_at);
        ofs.write(bingo_at_char, sizeof(result.bingo_at));
    }
    ofs.close();

	return EXIT_SUCCESS;
}
