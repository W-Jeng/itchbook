#include "mapped_file.h"
#include "profiler.h"
#include "reader.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "../data/short.bin";

    try {
        itchbook::MappedFile f(path);
        itchbook::Reader reader;

        auto t0 = std::chrono::steady_clock::now();
        reader.run(f);

        double ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - t0).count();
        std::cout << "Time taken: " << ms << " ms\n";
        std::cout << "\n[Event stats]\n" << reader.stats() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
