#include "collection.h"
#include "../common/log.h"
#include "../common/serialization.h" // For calculating document size accurately
#include "sstable.h" // For loading from disk
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::find_if
#include "lsm_tree.h" // For LSMTree pointer
#include <filesystem>
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

Collection::Collection(LSMTree* parent_db, const std::string& path)
    : Memtable(), parent_db_(parent_db), path_(path), indexer_(std::make_unique<Indexer>()) {
    if (!path_.empty()) {
        load_indexes();
    }
}

// This constructor is redundant but kept for compatibility just in case.
Collection::Collection(const std::string& path, LSMTree* parent_db)
    : Memtable(), parent_db_(parent_db), path_(path), indexer_(std::make_unique<Indexer>()) {
    load_indexes();
}

void Collection::load_indexes() {
    if (path_.empty()) return;
    try {
        LOG_INFO("Loading indexes for collection from path: " + path_);
        indexer_->load_indexes(path_);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load indexes for collection at " + path_ + ": " + e.what());
    }
}

void Collection::save_indexes() {
    if (path_.empty()) return;
    try {
        LOG_INFO("Saving indexes for collection to path: " + path_);
        indexer_->save_indexes(path_);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save indexes for collection at " + path_ + ": " + e.what());
    }
}

void Collection::set_schema(const TissDB::Schema& schema) {
    schema_ = schema;
}

void Collection::create_index(const std::vector<std::string>& field_names, bool is_unique) {
    // Correctly call the Indexer's create_index with the default type (String)
    indexer_->create_index(field_names, is_unique, IndexType::String);
    // Bulk-load existing data into the new index
    for (const auto& pair : data) {
        if (pair.second) { // If it's a document, not a tombstone
            try {
                indexer_->update_indexes(pair.first, *pair.second);
            } catch (const std::runtime_error& e) {
                LOG_ERROR("Error bulk-loading data for key " + pair.first + " into new index: " + e.what());
            }
        }
    }
    save_indexes();
}

bool Collection::has_index(const std::vector<std::string>& field_names) const {
    return indexer_->has_index(field_names);
}

std::vector<std::vector<std::string>> Collection::get_available_indexes() const {
    return indexer_->get_available_indexes();
}

std::vector<std::string> Collection::find_by_index(const std::vector<std::string>& field_names, const std::vector<std::string>& values) const {
    std::vector<Value> value_variants;
    for(const auto& v : values) {
        value_variants.push_back(v);
    }
    return indexer_->find_by_index(field_names, value_variants);
}

void Collection::put(const std::string& key, const Document& doc) {
    const std::string& pk_field = schema_.get_primary_key();
    if (!pk_field.empty()) {
        if (get_value(doc, pk_field) == nullptr) {
            throw std::runtime_error("Primary key field '" + pk_field + "' is missing.");
        }
    }

    if (parent_db_) {
        for (const auto& fk : schema_.get_foreign_keys()) {
            const Value* fk_value_ptr = get_value(doc, fk.field_name);
            if (fk_value_ptr) {
                try {
                    std::string fk_value_str = TissDB::Query::value_to_string(*fk_value_ptr);
                    std::vector<Value> search_values;
                    search_values.push_back(fk_value_str);
                    auto results = parent_db_->find_by_index(fk.referenced_collection, {fk.referenced_field}, search_values);
                    if (results.empty()) {
                        throw std::runtime_error("Foreign key constraint violated on field '" + fk.field_name + "'. No matching document in referenced collection '" + fk.referenced_collection + "'.");
                    }
                } catch (const std::runtime_error& e) {
                    throw std::runtime_error("Foreign key constraint error: " + std::string(e.what()));
                }
            }
        }
    }

    auto it = data.find(key);
    size_t old_value_size = 0;

    if (it != data.end() && it->second) {
        indexer_->remove_from_indexes(key, *it->second);
        old_value_size = TissDB::serialize(*(it->second)).size();
    } else {
        estimated_size += key.size();
    }

    auto new_doc_ptr = std::make_shared<Document>(doc);
    indexer_->update_indexes(key, *new_doc_ptr);

    size_t new_value_size = TissDB::serialize(*new_doc_ptr).size();

    estimated_size -= old_value_size;
    estimated_size += new_value_size;

    data[key] = new_doc_ptr;
}

bool Collection::del(const std::string& key) {
    LOG_DEBUG("DELETE key: " + key);
    auto it = data.find(key);
    if (it == data.end() || !it->second) {
        return false;
    }

    size_t old_value_size = TissDB::serialize(*(it->second)).size();
    estimated_size -= old_value_size;

    indexer_->remove_from_indexes(key, *it->second);

    it->second = nullptr;
    return true;
}

std::optional<std::shared_ptr<Document>> Collection::get(const std::string& key) {
    LOG_DEBUG("GET key: " + key);
    auto it = data.find(key);
    if (it == data.end()) {
        return std::nullopt;
    }

    if (!it->second) {
        return it->second;
    }

    auto doc_copy = std::make_shared<Document>(*it->second);
    doc_copy->id = key;
    return doc_copy;
}

std::vector<Document> Collection::scan() const {
    LOG_DEBUG("SCAN collection");
    std::vector<Document> documents;
    for (const auto& pair : data) {
        if (pair.second) {
            Document doc_with_id = *pair.second;
            doc_with_id.id = pair.first;
            documents.push_back(doc_with_id);
        } else {
            Document tombstone;
            tombstone.id = pair.first;
            documents.push_back(tombstone);
        }
    }
    return documents;
}

} // namespace Storage
} // namespace TissDB
