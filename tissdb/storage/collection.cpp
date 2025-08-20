#include "collection.h"
#include "../common/log.h"
#include "../common/serialization.h" // For calculating document size accurately
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::find_if
#include "lsm_tree.h" // For LSMTree pointer
#include "../query/executor_common.h" // For value_to_string

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

void Collection::create_index(const std::vector<std::string>& field_names, bool is_unique) {
    indexer_.create_index(field_names, is_unique);
    // Bulk-load existing data into the new index
    for (const auto& pair : data) {
        if (pair.second) { // If it's a document, not a tombstone
            try {
                indexer_.update_indexes(pair.first, *pair.second);
            } catch (const std::runtime_error& e) {
                // If a uniqueness constraint is violated during index creation,
                // it's a critical error. We should probably roll back the index creation.
                // For now, we'll log the error and continue, but this is not ideal.
                LOG_ERROR("Error bulk-loading data for key " + pair.first + " into new index: " + e.what());
                // In a real scenario, you'd want to delete the index that was just created.
            }
        }
    }
}

void Collection::shutdown() {
    // TODO: Implement shutdown
}

bool Collection::has_index(const std::vector<std::string>& field_names) const {
    return indexer_.has_index(field_names);
}

std::vector<std::vector<std::string>> Collection::get_available_indexes() const {
    return indexer_.get_available_indexes();
}

std::vector<std::string> Collection::find_by_index(const std::vector<std::string>& field_names, const std::vector<std::string>& values) const {
    return indexer_.find_by_index(field_names, values);
}

void Collection::put(const std::string& key, const Document& doc) {
    LOG_DEBUG("PUT key: " + key);

    // =================================================================
    // Schema Validation & Constraint Checking
    // =================================================================

    // 1. Primary Key Presence Check
    const std::string& pk_field = schema_.get_primary_key();
    if (!pk_field.empty()) {
        const Value* pk_value_ptr = get_value(doc, pk_field);
        if (pk_value_ptr == nullptr) {
            throw std::runtime_error("Primary key field '" + pk_field + "' is missing.");
        }
    }

    // 2. Foreign Key Existence Check (using indexes)
    if (parent_db_) {
        for (const auto& fk : schema_.get_foreign_keys()) {
            const Value* fk_value_ptr = get_value(doc, fk.field_name);
            if (fk_value_ptr) {
                try {
                    std::string fk_value_str = TissDB::Query::value_to_string(*fk_value_ptr);
                    auto results = parent_db_->find_by_index(fk.referenced_collection, {fk.referenced_field}, {fk_value_str});
                    if (results.empty()) {
                        throw std::runtime_error("Foreign key constraint violated on field '" + fk.field_name + "'. No matching document in referenced collection '" + fk.referenced_collection + "'.");
                    }
                } catch (const std::runtime_error& e) {
                    throw std::runtime_error("Foreign key constraint error: " + std::string(e.what()));
                }
            }
        }
    }

    // =================================================================
    // Index Maintenance & Data Insertion
    // =================================================================

    auto it = data.find(key);
    size_t old_value_size = 0;

    // If document exists, remove its old version from indexes first
    if (it != data.end() && it->second) {
        indexer_.remove_from_indexes(key, *it->second);
        old_value_size = TissDB::serialize(*(it->second)).size();
    } else {
        estimated_size += key.size();
    }

    // Update indexes with the new document content
    // This will throw on unique constraint violation (including PK)
    indexer_.update_indexes(key, doc);

    // Create the new document and calculate its size.
    auto new_doc_ptr = std::make_shared<Document>(doc);
    size_t new_value_size = TissDB::serialize(*new_doc_ptr).size();

    // Update the total estimated size.
    estimated_size -= old_value_size;
    estimated_size += new_value_size;

    // Insert the new document into the map.
    data[key] = new_doc_ptr;
}

bool Collection::del(const std::string& key) {
    LOG_DEBUG("DELETE key: " + key);
    auto it = data.find(key);
    if (it == data.end() || !it->second) {
        // Key doesn't exist or is already a tombstone
        return false;
    }

    // Document exists, so we proceed with deletion.
    size_t old_value_size = TissDB::serialize(*(it->second)).size();
    estimated_size -= old_value_size;

    // Remove the document from all indexes before marking it as deleted.
    indexer_.remove_from_indexes(key, *it->second);

    // Insert a null pointer as a tombstone marker.
    it->second = nullptr;
    return true;
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
            Document doc_with_id = *pair.second;
            doc_with_id.id = pair.first;
            documents.push_back(doc_with_id);
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
