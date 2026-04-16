#ifndef RMA_ERROR_TYPES_HPP
#define RMA_ERROR_TYPES_HPP

#include <cstdint>
#include <cstddef>

namespace rma {

enum class ErrorType : uint8_t {
    // Runtime errors
    LOGIC_BUG        = 0,
    OUT_OF_BOUNDS    = 1,
    NULL_POINTER     = 2,
    FLOAT_PRECISION  = 3,

    // Parse/compile errors
    ILLEGAL_INDENT   = 4,
    UNCLOSED_STRING  = 5,
    UNDEFINED_OBJ    = 6,
    UNDEFINED_METHOD = 7,
    ILLEGAL_KEYWORD  = 8,
    MISSING_OPERATOR = 9,

    // General categories
    DEBUG_INFO       = 10,
    WARNING          = 11,
    INFO             = 12,
    FATAL            = 13,
    CUSTOM           = 14,

    ERROR_TYPE_COUNT = 15
};

inline const char* error_type_name(ErrorType t) {
    static const char* names[] = {
        "LOGIC_BUG", "OUT_OF_BOUNDS", "NULL_POINTER", "FLOAT_PRECISION",
        "ILLEGAL_INDENT", "UNCLOSED_STRING", "UNDEFINED_OBJ", "UNDEFINED_METHOD",
        "ILLEGAL_KEYWORD", "MISSING_OPERATOR", "DEBUG_INFO", "WARNING",
        "INFO", "FATAL", "CUSTOM"
    };
    auto idx = static_cast<size_t>(t);
    return idx < sizeof(names)/sizeof(names[0]) ? names[idx] : "UNKNOWN";
}

inline char error_type_key(ErrorType t) {
    // Keys 0-9 for first 10 types, then a-e for remaining
    auto idx = static_cast<uint8_t>(t);
    if (idx < 10) return '0' + idx;
    return 'a' + (idx - 10);
}

inline ErrorType key_to_error_type(char key) {
    if (key >= '0' && key <= '9') return static_cast<ErrorType>(key - '0');
    if (key >= 'a' && key <= 'e') return static_cast<ErrorType>(10 + key - 'a');
    return ErrorType::ERROR_TYPE_COUNT; // invalid
}

} // namespace rma

#endif // RMA_ERROR_TYPES_HPP
