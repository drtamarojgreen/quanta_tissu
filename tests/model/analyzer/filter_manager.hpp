#ifndef RMA_FILTER_MANAGER_HPP
#define RMA_FILTER_MANAGER_HPP

#include "error_types.hpp"
#include <array>
#include <cstdio>
#include <mutex>

namespace rma {

class FilterManager {
public:
    // Non-copyable (has mutex), but could be moved if needed
    FilterManager(const FilterManager&) = delete;
    FilterManager& operator=(const FilterManager&) = delete;

    FilterManager() {
        // All filters enabled by default
        for (size_t i = 0; i < active_filters_.size(); ++i) {
            active_filters_[i] = true;
        }
    }

    // Toggle filter by keyboard key
    bool toggle(char key) {
        ErrorType t = key_to_error_type(key);
        if (t == ErrorType::ERROR_TYPE_COUNT) return false;

        std::lock_guard<std::mutex> lock(mtx_);
        size_t idx = static_cast<size_t>(t);
        active_filters_[idx] = !active_filters_[idx];
        return true;
    }

    // Check if error type should be displayed
    bool shouldDisplay(ErrorType t) const {
        size_t idx = static_cast<size_t>(t);
        if (idx >= active_filters_.size()) return true;

        std::lock_guard<std::mutex> lock(mtx_);
        return active_filters_[idx];
    }

    // Enable specific filter
    void enable(ErrorType t) {
        size_t idx = static_cast<size_t>(t);
        if (idx >= active_filters_.size()) return;

        std::lock_guard<std::mutex> lock(mtx_);
        active_filters_[idx] = true;
    }

    // Disable specific filter
    void disable(ErrorType t) {
        size_t idx = static_cast<size_t>(t);
        if (idx >= active_filters_.size()) return;

        std::lock_guard<std::mutex> lock(mtx_);
        active_filters_[idx] = false;
    }

    // Enable all filters
    void enableAll() {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& f : active_filters_) f = true;
    }

    // Disable all filters
    void disableAll() {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& f : active_filters_) f = false;
    }

    // Print current filter status to terminal
    void printStatus() const {
        std::lock_guard<std::mutex> lock(mtx_);

        std::printf("\n--- Filter Status (press key to toggle) ---\n");
        for (size_t i = 0; i < static_cast<size_t>(ErrorType::ERROR_TYPE_COUNT); ++i) {
            ErrorType t = static_cast<ErrorType>(i);
            char key = error_type_key(t);
            const char* name = error_type_name(t);
            const char* status = active_filters_[i] ? "ON " : "OFF";

            std::printf("  [%c] %-16s %s\n", key, name, status);
        }
        std::printf("  [A] Enable ALL\n");
        std::printf("  [Z] Disable ALL\n");
        std::printf("  [H] Show this help\n");
        std::printf("  [Q] Quit analyzer\n");
        std::printf("-------------------------------------------\n\n");
    }

private:
    mutable std::mutex mtx_;
    std::array<bool, static_cast<size_t>(ErrorType::ERROR_TYPE_COUNT)> active_filters_;
};

} // namespace rma

#endif // RMA_FILTER_MANAGER_HPP
