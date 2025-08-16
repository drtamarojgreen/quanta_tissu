#include "binary_stream_buffer.h"

namespace TissDB {

// Constructor for input stream
BinaryStreamBuffer::BinaryStreamBuffer(std::istream& is) : is_ptr_(&is), os_ptr_(nullptr) {}
// Constructor for output stream
BinaryStreamBuffer::BinaryStreamBuffer(std::ostream& os) : is_ptr_(nullptr), os_ptr_(&os) {}

// Read/Write primitive types


// Read/Write length-prefixed strings
std::string BinaryStreamBuffer::read_string() {
    size_t len;
    read(len);
    if (len == 0) return "";

    if (len > MAX_STRING_LEN) {
        throw std::runtime_error("Attempted to read excessively large string (len: " + std::to_string(len) + ").");
    }

    std::string str(len, '\0');
    is_ptr_->read(&str[0], len);
    if (!(*is_ptr_)) throw std::runtime_error("Failed to read string data.");
    return str;
}

void BinaryStreamBuffer::write_string(const std::string& str) {
    size_t len = str.size();
    write(len);
    os_ptr_->write(str.data(), len);
    if (!(*os_ptr_)) throw std::runtime_error("Failed to write string data.");
}

// Read/Write length-prefixed byte vectors
std::vector<uint8_t> BinaryStreamBuffer::read_bytes() {
    size_t len;
    read(len);
    if (len == 0) return {};

    if (len > MAX_BYTES_LEN) {
        throw std::runtime_error("Attempted to read excessively large byte array (len: " + std::to_string(len) + ").");
    }

    std::vector<uint8_t> data(len);
    is_ptr_->read(reinterpret_cast<char*>(data.data()), len);
    if (!(*is_ptr_)) throw std::runtime_error("Failed to read byte array data.");
    return data;
}

void BinaryStreamBuffer::write_bytes(const std::vector<uint8_t>& data) {
    size_t len = data.size();
    write(len);
    os_ptr_->write(reinterpret_cast<const char*>(data.data()), len);
    if (!(*os_ptr_)) throw std::runtime_error("Failed to write byte array data.");
}

// Read bytes given a pre-read length, with sanity checks
std::vector<uint8_t> BinaryStreamBuffer::read_bytes_with_length(size_t len) {
    if (!is_ptr_) throw std::runtime_error("Stream buffer not initialized for reading.");
    if (len == 0) return {};

    if (len > MAX_BYTES_LEN) {
        throw std::runtime_error("Attempted to read excessively large byte array (len: " + std::to_string(len) + ").");
    }

    std::vector<uint8_t> data(len);
    is_ptr_->read(reinterpret_cast<char*>(data.data()), len);
    if (!(*is_ptr_)) throw std::runtime_error("Failed to read byte array data with pre-read length.");
    return data;
}

// Check stream state
bool BinaryStreamBuffer::good() const { 
    if (is_ptr_) return is_ptr_->good();
    if (os_ptr_) return os_ptr_->good();
    return false; // Neither stream is active
}
bool BinaryStreamBuffer::eof() const { 
    if (is_ptr_) return is_ptr_->eof();
    return false; // Not an input stream
}

} // namespace TissDB
