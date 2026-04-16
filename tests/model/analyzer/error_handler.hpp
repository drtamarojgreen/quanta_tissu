#ifndef RMA_ERROR_HANDLER_HPP
#define RMA_ERROR_HANDLER_HPP

#include "error_wrapper.hpp"
#include "error_types.hpp"
#include "rpc_protocol.hpp"
#include <cstdio>
#include <cstdarg>

namespace rma {

// Singleton error handler for host application
class ErrorHandler {
public:
    // Delete copy/move - singleton
    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;

    static ErrorHandler& instance() {
        static ErrorHandler inst;
        return inst;
    }

    bool init(uint32_t session_id = 0) {
        if (initialized_) return true;
        initialized_ = rpc_.init_host(session_id);
        return initialized_;
    }

    bool is_initialized() const { return initialized_; }

    // Primary templated report function
    template<typename T>
    void report(ErrorType type, const T& value, const char* file, int line,
                const char* msg = "") noexcept {
        if (!initialized_) return;

        try {
            ErrorWrapper<T> wrapper(type, value, file, line, msg);
            auto data = wrapper.serialize();
            rpc_.send(data.data(), data.size(), ErrorWrapper<T>::type_identifier());
        } catch (...) {
            // Never throw from error handler
        }
    }

    // Convenience overloads for common types
    void report_int(ErrorType type, int value, const char* file, int line,
                    const char* msg = "") noexcept {
        report<int>(type, value, file, line, msg);
    }

    void report_ptr(ErrorType type, const void* ptr, const char* file, int line,
                    const char* msg = "") noexcept {
        report<uint64_t>(type, reinterpret_cast<uint64_t>(ptr), file, line, msg);
    }

    void report_float(ErrorType type, double value, const char* file, int line,
                      const char* msg = "") noexcept {
        report<double>(type, value, file, line, msg);
    }

    // Simple message report (no typed value)
    void report_msg(ErrorType type, const char* file, int line,
                    const char* fmt, ...) noexcept {
        if (!initialized_) return;

        char msg[512];
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);

        report<int>(type, 0, file, line, msg);
    }

    void shutdown() {
        // RpcProtocol destructor handles cleanup
        initialized_ = false;
    }

private:
    ErrorHandler() : initialized_(false) {}
    ~ErrorHandler() { shutdown(); }

    RpcProtocol rpc_;
    bool initialized_;
};

} // namespace rma

// Convenience macros for host application
#define RMA_INIT(session_id) rma::ErrorHandler::instance().init(session_id)

#define RMA_ERROR(type, msg) \
    rma::ErrorHandler::instance().report_msg(type, __FILE__, __LINE__, "%s", msg)

#define RMA_ERROR_FMT(type, fmt, ...) \
    rma::ErrorHandler::instance().report_msg(type, __FILE__, __LINE__, fmt, __VA_ARGS__)

#define RMA_ERROR_VAL(type, val, msg) \
    rma::ErrorHandler::instance().report(type, val, __FILE__, __LINE__, msg)

#define RMA_DEBUG(msg) RMA_ERROR(rma::ErrorType::DEBUG_INFO, msg)
#define RMA_WARNING(msg) RMA_ERROR(rma::ErrorType::WARNING, msg)
#define RMA_INFO(msg) RMA_ERROR(rma::ErrorType::INFO, msg)

#define RMA_NULL_CHECK(ptr) do { \
    if (!(ptr)) { \
        rma::ErrorHandler::instance().report_ptr( \
            rma::ErrorType::NULL_POINTER, ptr, __FILE__, __LINE__, #ptr " is null"); \
    } \
} while(0)

#define RMA_BOUNDS_CHECK(idx, size) do { \
    if ((idx) >= (size)) { \
        rma::ErrorHandler::instance().report_int( \
            rma::ErrorType::OUT_OF_BOUNDS, static_cast<int>(idx), __FILE__, __LINE__, \
            "Index " #idx " out of bounds"); \
    } \
} while(0)

#endif // RMA_ERROR_HANDLER_HPP
