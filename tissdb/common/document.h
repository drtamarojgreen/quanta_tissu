#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <chrono>
#include <memory>

namespace TissDB {

class Element;
struct Array;
struct Object;

// Primitive types supported by TissDB
using Number = double; // Using double to handle both integer and float values
using Boolean = bool;

struct Date {
    uint16_t year;
    uint8_t month;
    uint8_t day;

    bool operator==(const Date& other) const {
        return year == other.year && month == other.month && day == other.day;
    }
     bool operator<(const Date& other) const {
        if (year < other.year) return true;
        if (year > other.year) return false;
        if (month < other.month) return true;
        if (month > other.month) return false;
        return day < other.day;
    }
};

struct Time {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

     bool operator==(const Time& other) const {
        return hour == other.hour && minute == other.minute && second == other.second;
    }
    bool operator<(const Time& other) const {
        if (hour < other.hour) return true;
        if (hour > other.hour) return false;
        if (minute < other.minute) return true;
        if (minute > other.minute) return false;
        return second < other.second;
    }
};

struct Timestamp {
    // Use a 64-bit integer to store microseconds since Unix epoch, UTC.
    // This provides high precision and avoids timezone ambiguity in storage.
    int64_t microseconds_since_epoch_utc;

    bool operator==(const Timestamp& other) const {
        return microseconds_since_epoch_utc == other.microseconds_since_epoch_utc;
    }
    bool operator<(const Timestamp& other) const {
        return microseconds_since_epoch_utc < other.microseconds_since_epoch_utc;
    }
    bool operator!=(const Timestamp& other) const { return !(*this == other); }
    bool operator>(const Timestamp& other) const { return other < *this; }
    bool operator<=(const Timestamp& other) const { return !(other < *this); }
    bool operator>=(const Timestamp& other) const { return !(*this < other); }
};

using DateTime = std::chrono::time_point<std::chrono::system_clock>;
using BinaryData = std::vector<uint8_t>;

using Value = std::variant<
    std::nullptr_t,
    std::string,
    Number,
    Boolean,
    Date,
    Time,
    Timestamp,
    BinaryData,
    std::vector<Element>,
    std::shared_ptr<Array>,
    std::shared_ptr<Object>
>;

inline bool operator==(const Value& lhs, const Value& rhs) {
    if (lhs.index() != rhs.index()) {
        return false;
    }
    if (lhs.valueless_by_exception()) {
        return true;
    }

    return std::visit(
        [](const auto& a, const auto& b) -> bool {
            using T = std::decay_t<decltype(a)>;
            using U = std::decay_t<decltype(b)>;

            if constexpr (std::is_same_v<T, U>) {
            if constexpr (std::is_same_v<T, std::shared_ptr<Array>>) {
                    if (a && b) return *a == *b;
                    return !a && !b;
            } else if constexpr (std::is_same_v<T, std::shared_ptr<Object>>) {
                    if (a && b) return *a == *b;
                    return !a && !b;
                } else {
                    return a == b;
                }
            } else {
                return false;
            }
        },
        lhs, rhs);
}

struct Array {
    std::vector<Value> values;
    bool operator==(const Array& other) const;
};

struct Object {
    std::map<std::string, Value> values;
    bool operator==(const Object& other) const;
};

// An Element is a key-value pair, representing a tag in the document.
class Element {
public:
    std::string key;
    Value value;

    bool operator==(const Element& other) const {
        return key == other.key && value == other.value;
    }
};

// A Document is the top-level object, identified by an ID and containing a collection of root elements.
class Document {
public:
    std::string id;
    std::vector<Element> elements;
    bool is_tombstone() const { return elements.empty() && !id.empty(); }

    bool operator==(const Document& other) const {
        return id == other.id && elements == other.elements;
    }
};

} // namespace TissDB