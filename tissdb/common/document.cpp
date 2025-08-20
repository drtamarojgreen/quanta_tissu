#include "document.h"
#include <iostream>

namespace TissDB {


bool Array::operator==(const Array& other) const {
    return values == other.values;
}

bool Object::operator==(const Object& other) const {
    return values == other.values;
}

} // namespace TissDB
