#ifndef RMA_FILE_BUFFER_HPP
#define RMA_FILE_BUFFER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <ctime>
#include <cstdio>

namespace rma {

class FileBuffer {
public:
    // Non-copyable (has mutex and file handle)
    FileBuffer(const FileBuffer&) = delete;
    FileBuffer& operator=(const FileBuffer&) = delete;

    FileBuffer() : flush_threshold_(100), total_entries_(0) {}

    explicit FileBuffer(const std::string& filepath, size_t flush_threshold = 100)
        : flush_threshold_(flush_threshold), total_entries_(0) {
        open(filepath);
    }

    ~FileBuffer() {
        flush();
        if (file_.is_open()) file_.close();
    }

    bool open(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(mtx_);

        if (file_.is_open()) file_.close();

        filepath_ = filepath;
        file_.open(filepath, std::ios::out | std::ios::app);

        if (file_.is_open()) {
            // Write session header
            std::time_t now = std::time(nullptr);
            char timebuf[64];
            std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S",
                          std::localtime(&now));
            file_ << "\n=== Analyzer Session Started: " << timebuf << " ===\n";
            file_.flush();
        }

        return file_.is_open();
    }

    void append(const std::string& entry) {
        std::lock_guard<std::mutex> lock(mtx_);

        buffer_.push_back(entry);
        ++total_entries_;

        if (buffer_.size() >= flush_threshold_) {
            flush_internal();
        }
    }

    void flush() {
        std::lock_guard<std::mutex> lock(mtx_);
        flush_internal();
    }

    size_t buffered_count() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return buffer_.size();
    }

    size_t total_entries() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return total_entries_;
    }

    bool is_open() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return file_.is_open();
    }

    void set_flush_threshold(size_t threshold) {
        std::lock_guard<std::mutex> lock(mtx_);
        flush_threshold_ = threshold;
    }

private:
    void flush_internal() {
        // Must be called with lock held
        if (!file_.is_open() || buffer_.empty()) return;

        for (const auto& entry : buffer_) {
            file_ << entry << '\n';
        }
        file_.flush();
        buffer_.clear();
    }

    mutable std::mutex mtx_;
    std::ofstream file_;
    std::string filepath_;
    std::vector<std::string> buffer_;
    size_t flush_threshold_;
    size_t total_entries_;
};

} // namespace rma

#endif // RMA_FILE_BUFFER_HPP
