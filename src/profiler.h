#pragma once
#include <cstdint>
#include <x86intrin.h>
#include <vector>
#include <cstddef>
#include <algorithm>


namespace itchbook {

template<std::size_t N>
class SampleBuffer {
public:
    SampleBuffer() {
        m_samples.reserve(N);
        m_samples.resize(N);    // pre-fault the pages
        m_samples.clear();      // go back 0-th index
    }

    void add(uint64_t s) {
        if (m_samples.size() < N) {
            m_samples.push_back(s > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(s));
            m_sorted = false;
        } else {
            ++m_dropped;
        }
    }

    uint32_t pct(double p) {
        if (size() == 0)
            return 0;

        if (!m_sorted) { 
            std::sort(m_samples.begin(), m_samples.end());
            m_sorted = true;
        }
        
        std::size_t index = std::size_t((p/100) * (size() -1));
        return m_samples[index];
    }

    const std::vector<uint32_t>& raw() const { return m_samples; }
    std::size_t size() const { return m_samples.size(); }
    std::size_t dropped() const { return m_dropped; }

private:
    std::vector<uint32_t> m_samples{}; 
    std::size_t m_dropped = 0;
    bool m_sorted = false;
};


template<std::size_t N>
class LatencyProbe {
public:
    [[gnu::always_inline]] inline void start() {
        unsigned aux;
        m_t0 = __rdtscp(&aux);
        _mm_lfence();
    }

    [[gnu::always_inline]] inline void stop() {
        unsigned aux;
        uint64_t t1 = __rdtscp(&aux);
        _mm_lfence();
        
        if (t1 >= m_t0)
            m_samples.add(t1-m_t0);
    }

    const SampleBuffer<N>& samples() const { return m_samples; }
    SampleBuffer<N>& samples() { return m_samples; }

private:
    SampleBuffer<N> m_samples;
    uint64_t m_t0{};
};


template <typename Site, bool Enabled, std::size_t N>
class Profiler {
public:
    [[gnu::always_inline]] void start(Site s) {
        if constexpr (Enabled) 
            m_probes[static_cast<std::size_t>(s)].start();
    }

    [[gnu::always_inline]] void stop(Site s) {
        if constexpr (Enabled) 
            m_probes[static_cast<std::size_t>(s)].stop();
    }
    
    SampleBuffer<N>& samples(Site s) {
        return m_probes[static_cast<std::size_t>(s)].samples();
    }

private:
    static constexpr std::size_t sites_count = static_cast<std::size_t>(Site::COUNT);
    std::array<LatencyProbe<N>, Enabled ? sites_count : 0> m_probes;
};

} 
