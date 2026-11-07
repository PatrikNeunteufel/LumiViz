/**
 * @file Pool.cpp
 * @brief InstrumentCore: `parallelJe` (aus IC `rechnung/gemeinsam/Werkzeug.cpp` `bb7e6d6`, dazu die Ausnahme)
 */
#include "InstrumentCore/Pool.hpp"

#include <algorithm>
#include <atomic>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

namespace lumi::kern
{

unsigned poolThreads()
{
    return std::clamp(std::thread::hardware_concurrency(), 1u, kPoolHoechstens);
}

void parallelJe(std::size_t anzahl, const std::function<void(std::size_t)>& aufgabe, unsigned threads)
{
    if (anzahl == 0) return;
    if (threads == 0) threads = poolThreads();
    threads = static_cast<unsigned>(std::min<std::size_t>(threads, anzahl));
    if (threads <= 1)
    {
        for (std::size_t i = 0; i < anzahl; ++i) aufgabe(i);
        return;
    }
    std::atomic<std::size_t> naechste{0};
    std::mutex schutz;
    std::exception_ptr erste;
    std::vector<std::thread> pool;
    pool.reserve(threads);
    for (unsigned t = 0; t < threads; ++t)
        pool.emplace_back([&] {
            for (std::size_t i = naechste++; i < anzahl; i = naechste++)
            {
                try
                {
                    aufgabe(i);
                }
                catch (...)
                {
                    const std::lock_guard<std::mutex> sperre(schutz);
                    if (!erste) erste = std::current_exception();
                }
            }
        });
    for (std::thread& t : pool) t.join();
    if (erste) std::rethrow_exception(erste);
}

}  // namespace lumi::kern
