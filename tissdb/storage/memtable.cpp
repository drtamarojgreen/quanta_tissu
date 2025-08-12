#include "memtable.h"

namespace TissDB {
namespace Storage {

Memtable::Memtable() : estimated_size(0) {}

void Memtable::put(const std::string& key, const Document& doc) {
    // Placeholder: In a real implementation, we would calculate the size
    // of the key and document to update estimated_size.
    (void)key;
    (void)doc;
}

void Memtable::del(const std::string& key) {
    // Placeholder: This would insert a tombstone marker.
    (void)key;
}

std::optional<std::shared_ptr<Document>> Memtable::get(const std::string& key) {
    // Placeholder.
    (void)key;
    return std::nullopt;
}

const std::map<std::string, std::shared_ptr<Document>>& Memtable::get_all() const {
    return data;
}

void Memtable::clear() {
    data.clear();
    estimated_size = 0;
}

size_t Memtable::approximate_size() const {
    return estimated_size;
}

} // namespace Storage
} // namespace TissDB
