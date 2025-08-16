#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace TissDB {

class BinaryStreamBuffer {
public:
    // Constructor for input stream
    explicit BinaryStreamBuffer(std::istream& is);
    // Constructor for output stream
    explicit BinaryStreamBuffer(std::ostream& os);

    // Read/Write primitive types
    template<typename T>
    void read(T& value) {
        if (!is_ptr_) throw std::runtime_error("Stream buffer not initialized for reading.");
        is_ptr_->read(reinterpret_cast<char*>(&value), sizeof(T));
        if (!(*is_ptr_)) throw std::runtime_error("Failed to read binary data.");
    }

    template<typename T>
    void write(const T& value) {
        if (!os_ptr_) throw std::runtime_error("Stream buffer not initialized for writing.");
        os_ptr_->write(reinterpret_cast<const char*>(&value), sizeof(T));
        if (!(*os_ptr_)) throw std::runtime_error("Failed to write binary data.");
    }

    // Read/Write length-prefixed strings
    std::string read_string();
    void write_string(const std::string& str);

    // Read/Write length-prefixed byte vectors
    std::vector<uint8_t> read_bytes();
    void write_bytes(const std::vector<uint8_t>& data);

    // Read bytes given a pre-read length, with sanity checks
    std::vector<uint8_t> read_bytes_with_length(size_t len);

    // Check stream state
    bool good() const;
    bool eof() const;

private:
    std::istream* is_ptr_;
    std::ostream* os_ptr_;

    // Max lengths for sanity checks
    static const size_t MAX_STRING_LEN = 10 * 1024 * 1024; // 10MB
    static const size_t MAX_BYTES_LEN = 100 * 1024 * 1024; // 100MB
};

} // namespace TissDB
