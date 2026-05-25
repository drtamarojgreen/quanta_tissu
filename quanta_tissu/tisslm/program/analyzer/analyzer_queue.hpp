#ifndef RMA_ANALYZER_QUEUE_HPP
#define RMA_ANALYZER_QUEUE_HPP

#include "error_wrapper.hpp"
#include <deque>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>

namespace rma {

class AnalyzerQueue {
public:
    // Non-copyable, non-movable - shared resource with mutex
    AnalyzerQueue(const AnalyzerQueue&) = delete;
    AnalyzerQueue& operator=(const AnalyzerQueue&) = delete;
    AnalyzerQueue(AnalyzerQueue&&) = delete;
    AnalyzerQueue& operator=(AnalyzerQueue&&) = delete;

    AnalyzerQueue() : shutdown_(false), max_size_(10000) {}

    explicit AnalyzerQueue(size_t max_size)
        : shutdown_(false), max_size_(max_size) {}

    ~AnalyzerQueue() {
        shutdown();
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto* node : queue_) {
            delete node;
        }
        queue_.clear();
    }

    // Enqueue - takes ownership of node
    bool enqueue(ErrorNodeBase* node) {
        if (!node) return false;

        std::lock_guard<std::mutex> lock(mtx_);
        if (shutdown_) {
            delete node;
            return false;
        }

        // Drop oldest if at capacity
        if (queue_.size() >= max_size_) {
            delete queue_.front();
            queue_.pop_front();
        }

        queue_.push_back(node);
        cv_.notify_one();
        return true;
    }

    // Templated enqueue for convenience
    template<typename T>
    bool enqueue(const ErrorWrapper<T>& error) {
        auto* node = new ErrorNode<T>(error);
        return enqueue(static_cast<ErrorNodeBase*>(node));
    }

    // Blocking dequeue with timeout
    ErrorNodeBase* dequeue(std::chrono::milliseconds timeout =
                           std::chrono::milliseconds(100)) {
        std::unique_lock<std::mutex> lock(mtx_);

        bool got_data = cv_.wait_for(lock, timeout, [this] {
            return !queue_.empty() || shutdown_;
        });

        if (!got_data || queue_.empty()) return nullptr;

        ErrorNodeBase* node = queue_.front();
        queue_.pop_front();
        return node;
    }

    // Non-blocking dequeue
    ErrorNodeBase* try_dequeue() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) return nullptr;

        ErrorNodeBase* node = queue_.front();
        queue_.pop_front();
        return node;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mtx_);
        shutdown_ = true;
        cv_.notify_all();
    }

    bool is_shutdown() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return shutdown_;
    }

private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::deque<ErrorNodeBase*> queue_;
    bool shutdown_;
    size_t max_size_;
};

} // namespace rma

#endif // RMA_ANALYZER_QUEUE_HPP
