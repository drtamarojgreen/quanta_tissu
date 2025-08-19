#include "document.h"
#include <iostream>

namespace TissDB {

// Custom equality check for our Value variant
bool operator==(const Value& lhs, const Value& rhs) {
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
                if constexpr (std::is_same_v<T, std::unique_ptr<Array>>) {
                    if (a && b) return *a == *b;
                    return !a && !b;
                } else if constexpr (std::is_same_v<T, std::unique_ptr<Object>>) {
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

bool Array::operator==(const Array& other) const {
    return values == other.values;
}

bool Object::operator==(const Object& other) const {
    return values == other.values;
}

} // namespace TissDB
