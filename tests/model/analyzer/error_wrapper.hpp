#ifndef RMA_ERROR_WRAPPER_HPP
#define RMA_ERROR_WRAPPER_HPP

#include "error_types.hpp"
#include <cstdint>
#include <cstring>
#include <chrono>
#include <string>
#include <vector>
#include <type_traits>

namespace rma {

// Forward declaration for serialization size calculation
template<typename T>
struct SerializedSize {
    static constexpr size_t value = sizeof(T);
};

template<>
struct SerializedSize<std::string> {
    static constexpr size_t header = sizeof(uint32_t);
    // Actual size computed at runtime
};

// Templated error wrapper - core of the framework
template<typename T>
class ErrorWrapper {
public:
    static_assert(std::is_trivially_copyable_v<T> || std::is_same_v<T, std::string>,
                  "ErrorWrapper<T> requires T to be trivially copyable or std::string");

    ErrorType type;
    T value;
    uint64_t timestamp;
    char source_file[256];
    int32_t source_line;
    char message[512];

    ErrorWrapper() : type(ErrorType::INFO), value{}, timestamp(0),
                     source_line(0) {
        source_file[0] = '\0';
        message[0] = '\0';
    }

    ErrorWrapper(ErrorType t, const T& val, const char* file, int line,
                 const char* msg = "")
        : type(t), value(val), source_line(line) {
        timestamp = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        std::strncpy(source_file, file, sizeof(source_file) - 1);
        source_file[sizeof(source_file) - 1] = '\0';
        std::strncpy(message, msg, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';
    }

    // Serialize for IPC - returns byte vector
    std::vector<uint8_t> serialize() const noexcept {
        std::vector<uint8_t> data;
        data.reserve(1024);

        // Type ID for deserialization
        uint32_t type_id = type_identifier();
        append_bytes(data, &type_id, sizeof(type_id));

        // Error type
        append_bytes(data, &type, sizeof(type));

        // Timestamp
        append_bytes(data, &timestamp, sizeof(timestamp));

        // Source info
        append_bytes(data, source_file, sizeof(source_file));
        append_bytes(data, &source_line, sizeof(source_line));
        append_bytes(data, message, sizeof(message));

        // Value - specialized handling
        serialize_value(data);

        return data;
    }

    // Deserialize from byte buffer
    static ErrorWrapper<T> deserialize(const uint8_t* data, size_t len) noexcept {
        ErrorWrapper<T> wrapper;
        if (len < min_serialized_size()) return wrapper;

        size_t offset = sizeof(uint32_t); // skip type_id, already validated

        std::memcpy(&wrapper.type, data + offset, sizeof(wrapper.type));
        offset += sizeof(wrapper.type);

        std::memcpy(&wrapper.timestamp, data + offset, sizeof(wrapper.timestamp));
        offset += sizeof(wrapper.timestamp);

        std::memcpy(wrapper.source_file, data + offset, sizeof(wrapper.source_file));
        offset += sizeof(wrapper.source_file);

        std::memcpy(&wrapper.source_line, data + offset, sizeof(wrapper.source_line));
        offset += sizeof(wrapper.source_line);

        std::memcpy(wrapper.message, data + offset, sizeof(wrapper.message));
        offset += sizeof(wrapper.message);

        wrapper.deserialize_value(data, len, offset);

        return wrapper;
    }

    static constexpr uint32_t type_identifier() noexcept {
        // Simple type ID based on sizeof and is_integral
        if constexpr (std::is_same_v<T, int>) return 1;
        else if constexpr (std::is_same_v<T, double>) return 2;
        else if constexpr (std::is_same_v<T, float>) return 3;
        else if constexpr (std::is_same_v<T, int64_t>) return 4;
        else if constexpr (std::is_same_v<T, uint64_t>) return 5;
        else if constexpr (std::is_same_v<T, char>) return 6;
        else if constexpr (std::is_pointer_v<T>) return 100;
        else return 0; // generic
    }

    static constexpr size_t min_serialized_size() noexcept {
        return sizeof(uint32_t) + sizeof(ErrorType) + sizeof(uint64_t) +
               256 + sizeof(int32_t) + 512;
    }

private:
    static void append_bytes(std::vector<uint8_t>& vec, const void* src,
                             size_t len) noexcept {
        const uint8_t* p = static_cast<const uint8_t*>(src);
        vec.insert(vec.end(), p, p + len);
    }

    void serialize_value(std::vector<uint8_t>& data) const noexcept {
        if constexpr (std::is_trivially_copyable_v<T>) {
            append_bytes(data, &value, sizeof(value));
        }
    }

    void deserialize_value(const uint8_t* data, size_t len,
                           size_t offset) noexcept {
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (offset + sizeof(value) <= len) {
                std::memcpy(&value, data + offset, sizeof(value));
            }
        }
    }
};

// Type-erased base for queue storage
class ErrorNodeBase {
public:
    virtual ~ErrorNodeBase() = default;
    virtual ErrorType get_type() const noexcept = 0;
    virtual uint64_t get_timestamp() const noexcept = 0;
    virtual const char* get_source_file() const noexcept = 0;
    virtual int32_t get_source_line() const noexcept = 0;
    virtual const char* get_message() const noexcept = 0;
    virtual std::string format() const = 0;
    virtual std::vector<uint8_t> serialize() const = 0;

    // No copy/move - these are heap allocated and queue-owned
    ErrorNodeBase(const ErrorNodeBase&) = delete;
    ErrorNodeBase& operator=(const ErrorNodeBase&) = delete;
    ErrorNodeBase(ErrorNodeBase&&) = delete;
    ErrorNodeBase& operator=(ErrorNodeBase&&) = delete;

protected:
    ErrorNodeBase() = default;
};

// Concrete error node holding typed wrapper
template<typename T>
class ErrorNode : public ErrorNodeBase {
public:
    ErrorWrapper<T> error;

    explicit ErrorNode(const ErrorWrapper<T>& e) : error(e) {}
    explicit ErrorNode(ErrorWrapper<T>&& e) : error(std::move(e)) {}

    ErrorType get_type() const noexcept override { return error.type; }
    uint64_t get_timestamp() const noexcept override { return error.timestamp; }
    const char* get_source_file() const noexcept override {
        return error.source_file;
    }
    int32_t get_source_line() const noexcept override {
        return error.source_line;
    }
    const char* get_message() const noexcept override { return error.message; }

    std::string format() const override {
        char buf[1024];
        std::snprintf(buf, sizeof(buf), "[%s] %s:%d - %s",
                      error_type_name(error.type),
                      error.source_file,
                      error.source_line,
                      error.message);
        return std::string(buf);
    }

    std::vector<uint8_t> serialize() const override {
        return error.serialize();
    }
};

} // namespace rma

#endif // RMA_ERROR_WRAPPER_HPP
