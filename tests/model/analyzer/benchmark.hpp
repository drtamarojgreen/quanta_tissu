#ifndef RMA_BENCHMARK_HPP
#define RMA_BENCHMARK_HPP

#include <chrono>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <cstdio>

namespace rma {

class Benchmark {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::nanoseconds;

    struct Stats {
        uint64_t count = 0;
        Duration total{0};
        Duration min{Duration::max()};
        Duration max{Duration::zero()};

        double avg_ms() const {
            if (count == 0) return 0.0;
            return std::chrono::duration<double, std::milli>(total).count() /
                   static_cast<double>(count);
        }

        double total_ms() const {
            return std::chrono::duration<double, std::milli>(total).count();
        }

        double min_ms() const {
            if (count == 0) return 0.0;
            return std::chrono::duration<double, std::milli>(min).count();
        }

        double max_ms() const {
            return std::chrono::duration<double, std::milli>(max).count();
        }
    };

    // Non-copyable
    Benchmark(const Benchmark&) = delete;
    Benchmark& operator=(const Benchmark&) = delete;

    Benchmark() = default;

    void start(const std::string& tag) {
        std::lock_guard<std::mutex> lock(mtx_);
        active_[tag] = Clock::now();
    }

    void stop(const std::string& tag) {
        auto end = Clock::now();

        std::lock_guard<std::mutex> lock(mtx_);
        auto it = active_.find(tag);
        if (it == active_.end()) return;

        Duration elapsed = std::chrono::duration_cast<Duration>(end - it->second);
        active_.erase(it);

        Stats& s = stats_[tag];
        s.count++;
        s.total += elapsed;
        if (elapsed < s.min) s.min = elapsed;
        if (elapsed > s.max) s.max = elapsed;
    }

    // RAII helper for scoped timing
    class ScopedTimer {
    public:
        ScopedTimer(Benchmark& bm, const std::string& tag)
            : bm_(bm), tag_(tag) {
            bm_.start(tag_);
        }
        ~ScopedTimer() {
            bm_.stop(tag_);
        }

        // Non-copyable/movable
        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;

    private:
        Benchmark& bm_;
        std::string tag_;
    };

    ScopedTimer scoped(const std::string& tag) {
        return ScopedTimer(*this, tag);
    }

    std::string report() const {
        std::lock_guard<std::mutex> lock(mtx_);

        std::string result;
        result.reserve(1024);

        result += "\n=== Benchmark Report ===\n";
        char buf[256];

        for (const auto& [tag, stats] : stats_) {
            std::snprintf(buf, sizeof(buf),
                "%-20s: count=%6lu  avg=%.4fms  min=%.4fms  max=%.4fms  total=%.2fms\n",
                tag.c_str(),
                static_cast<unsigned long>(stats.count),
                stats.avg_ms(),
                stats.min_ms(),
                stats.max_ms(),
                stats.total_ms());
            result += buf;
        }

        result += "========================\n";
        return result;
    }

    void print_report() const {
        std::printf("%s", report().c_str());
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mtx_);
        stats_.clear();
        active_.clear();
    }

    const Stats* get_stats(const std::string& tag) const {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = stats_.find(tag);
        return it != stats_.end() ? &it->second : nullptr;
    }

private:
    mutable std::mutex mtx_;
    std::map<std::string, TimePoint> active_;
    std::map<std::string, Stats> stats_;
};

// Macro for convenient scoped benchmarking
#define RMA_BENCHMARK_SCOPE(bm, tag) \
    auto _rma_bm_scope_##__LINE__ = (bm).scoped(tag)

} // namespace rma

#endif // RMA_BENCHMARK_HPP
