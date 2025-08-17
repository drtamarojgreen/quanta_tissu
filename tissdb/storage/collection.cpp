#include "collection.h"
#include "../common/serialization.h" // For calculating document size accurately

namespace TissDB {
namespace Storage {

Collection::Collection() : estimated_size(0) {}

Collection::Collection(const std::string& path) : estimated_size(0) {
    // TODO: Implement loading collection from path
}

void Collection::set_schema(const TissDB::Schema& schema) {
    schema_ = schema;
}

void Collection::create_index(const std::vector<std::string>& field_names) {
    // TODO: Implement index creation
}

void Collection::shutdown() {
    // TODO: Implement shutdown
}

void Collection::put(const std::string& key, const Document& doc) {
    // To accurately track memory usage, we account for the change in size.
    size_t old_value_size = 0;
    auto it = data.find(key);
    if (it != data.end()) {
        // If the key already exists, find the size of the old value.
        if (it->second) { // If it's a document, not a tombstone
            old_value_size = TissDB::serialize(*(it->second)).size();
        }
    } else {
        // If the key is new, it adds the key's size to the total.
        estimated_size += key.size();
    }

    // Create the new document and calculate its size.
    auto new_doc_ptr = std::make_shared<Document>(doc);
    size_t new_value_size = TissDB::serialize(*new_doc_ptr).size();

    // Update the total estimated size.
    estimated_size -= old_value_size;
    estimated_size += new_value_size;

    // Insert the new document into the map.
    data[key] = new_doc_ptr;
}

void Collection::del(const std::string& key) {
    size_t old_value_size = 0;
    auto it = data.find(key);
    if (it != data.end()) {
        // If the key exists, get the size of the document being replaced.
        if (it->second) {
            old_value_size = TissDB::serialize(*(it->second)).size();
        }
    } else {
        // If the key is new, it adds its own size.
        estimated_size += key.size();
    }

    // A tombstone has no value, so the new value size is 0.
    estimated_size -= old_value_size;

    // Insert a null pointer as a tombstone marker.
    data[key] = nullptr;
}

std::optional<std::shared_ptr<Document>> Collection::get(const std::string& key) {
    auto it = data.find(key);
    if (it == data.end()) {
        // The key is not in the collection at all.
        return std::nullopt;
    }
    // The key is in the collection. The value could be a document or a tombstone (nullptr).
    return it->second;
}

const std::map<std::string, std::shared_ptr<Document>>& Collection::get_all() const {
    return data;
}

void Collection::clear() {
    data.clear();
    estimated_size = 0;
}


size_t Collection::approximate_size() const {
    return estimated_size;
}

std::vector<Document> Collection::scan() const {
    std::vector<Document> documents;
    for (const auto& pair : data) {
        if (pair.second) { // If it's a document, not a tombstone
            documents.push_back(*pair.second);
        } else {
            // Tombstone
            Document tombstone;
            tombstone.id = pair.first;
            documents.push_back(tombstone);
        }
    }
    return documents;
}


} // namespace Storage
} // namespace TissDB
