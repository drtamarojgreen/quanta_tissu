#include "collection.h"
#include "../common/log.h"
#include "../common/serialization.h" // For calculating document size accurately
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::find_if
#include "lsm_tree.h" // For LSMTree pointer

// Helper function to get a value from a document
const TissDB::Value* get_value(const TissDB::Document& doc, const std::string& key) {
    auto it = std::find_if(doc.elements.begin(), doc.elements.end(),
                           [&](const TissDB::Element& elem) { return elem.key == key; });
    if (it != doc.elements.end()) {
        return &it->value;
    }
    return nullptr;
}

namespace TissDB {
namespace Storage {

Collection::Collection(LSMTree* parent_db) : estimated_size(0), parent_db_(parent_db) {}

Collection::Collection(const std::string& path, LSMTree* parent_db) : estimated_size(0), parent_db_(parent_db) {
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
    LOG_DEBUG("PUT key: " + key);

    // =================================================================
    // Primary Key Enforcement
    // =================================================================
    const std::string& pk_field = schema_.get_primary_key();
    if (!pk_field.empty()) {
        // 1. Find the primary key value in the new document
        const Value* pk_value_ptr = get_value(doc, pk_field);

        // 2. Check for presence
        if (pk_value_ptr == nullptr) {
            throw std::runtime_error("Primary key field '" + pk_field + "' is missing.");
        }

        // 3. Check for nullness (a string is used for the key)
        if (std::holds_alternative<std::string>(*pk_value_ptr) && std::get<std::string>(*pk_value_ptr).empty()) {
            throw std::runtime_error("Primary key value for field '" + pk_field + "' cannot be empty.");
        }

        // 4. Check for uniqueness (O(N) scan)
        // TODO: This should be replaced with an index lookup for performance.
        for (const auto& pair : data) {
            if (pair.first != key && pair.second) { // Exclude the document being updated and tombstones
                const Value* existing_pk_value = get_value(*pair.second, pk_field);
                if (existing_pk_value && *existing_pk_value == *pk_value_ptr) {
                    throw std::runtime_error("Primary key constraint violated. Value for field '" + pk_field + "' already exists.");
                }
            }
        }
    }

    // =================================================================
    // Foreign Key Enforcement
    // =================================================================
    if (parent_db_) {
        for (const auto& fk : schema_.get_foreign_keys()) {
            const Value* fk_value_ptr = get_value(doc, fk.field_name);

            // Only check the constraint if the FK value is present
            if (fk_value_ptr) {
                // This assumes the referenced field is a primary key or has a unique index.
                // This check is O(M) where M is the number of documents in the referenced collection.
                // TODO: This should be replaced with an index lookup for performance.
                try {
                    const Collection& referenced_collection = parent_db_->get_collection(fk.referenced_collection);
                    const auto& referenced_data = referenced_collection.get_all();

                    bool found = false;
                    for (const auto& pair : referenced_data) {
                        if (pair.second) { // Exclude tombstones
                            const Value* referenced_value = get_value(*pair.second, fk.referenced_field);
                            if (referenced_value && *referenced_value == *fk_value_ptr) {
                                found = true;
                                break;
                            }
                        }
                    }

                    if (!found) {
                        throw std::runtime_error("Foreign key constraint violated on field '" + fk.field_name + "'. No matching value in referenced collection '" + fk.referenced_collection + "'.");
                    }
                } catch (const std::runtime_error& e) {
                    // This could be thrown by get_collection if the collection doesn't exist
                    throw std::runtime_error("Foreign key constraint error: " + std::string(e.what()));
                }
            }
        }
    }

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
    LOG_DEBUG("DELETE key: " + key);
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
    LOG_DEBUG("GET key: " + key);
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
    LOG_DEBUG("SCAN collection");
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
