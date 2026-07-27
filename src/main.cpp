#include "mapped_file.h"
#include "profiler.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "../data/short.bin";

    try {
        itchbook::MappedFile f(path);
        const std::byte* const begin = f.data();
        const std::byte* p   = begin;
        const std::byte* end = begin + f.size();
        std::size_t n = 0;
        itchbook::LatencyProbe<1'000'000> lp;
        auto t0 = std::chrono::steady_clock::now();

        while (end - p >= 2) {
            const uint16_t len = (
                (uint16_t(std::to_integer<uint8_t>(p[0])) << 8)
                | std::to_integer<uint8_t>(p[1])
            );

            if (static_cast<std::size_t>(end - p) < 2u + len) 
                break;
            
            // process 
            lp.start();
            p += 2 + len;
            lp.stop();
            ++n;
        }

        auto t1 = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0);
        std::cout << "Time taken: " << duration.count() << " ms\n";
        std::cout << "messages: " << n << "\n";
        std::cout << "lp p99.9: " << lp.samples().pct(99.9) << "\n";

        if (p != end) {
            std::cerr << "trailing bytes: " << (end - p)
                      << " at offset " << (p - begin) << "\n";
            return 1;
        }
        std::cout << "length chain terminates exactly on EOF\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
