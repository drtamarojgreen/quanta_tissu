#include "indexer.h"
#include <algorithm>

namespace TissDB {
namespace Storage {

// --- BTreeIndex placeholder implementation ---

void BTreeIndex::add(const std::string& value, const std::string& document_id) {
    // In a real B-Tree, this would be a more complex operation.
    index_data[value].push_back(document_id);
}

void BTreeIndex::remove(const std::string& value, const std::string& document_id) {
    auto it = index_data.find(value);
    if (it != index_data.end()) {
        auto& doc_ids = it->second;
        doc_ids.erase(std::remove(doc_ids.begin(), doc_ids.end(), document_id), doc_ids.end());
        if (doc_ids.empty()) {
            index_data.erase(it);
        }
    }
}

std::vector<std::string> BTreeIndex::find(const std::string& value) const {
    auto it = index_data.find(value);
    if (it != index_data.end()) {
        return it->second;
    }
    return {};
}

// --- Indexer placeholder implementation ---

Indexer::Indexer() = default;

void Indexer::create_index(const std::string& field_name) {
    if (indexes.find(field_name) == indexes.end()) {
        indexes[field_name] = std::make_unique<BTreeIndex>();
    }
}

bool Indexer::has_index(const std::string& field_name) const {
    return indexes.count(field_name) > 0;
}

void Indexer::update_indexes(const std::string& document_id, const Document& doc) {
    // Placeholder: This would iterate through the document's fields,
    // and for each field that has an index, it would call `add()` on the
    // corresponding BTreeIndex.
    (void)document_id;
    (void)doc;
}

void Indexer::remove_from_indexes(const std::string& document_id, const Document& doc) {
    // Placeholder: Similar to update, but would call `remove()`.
    (void)document_id;
    (void)doc;
}

std::vector<std::string> Indexer::find_by_index(const std::string& field_name, const std::string& value) const {
    auto it = indexes.find(field_name);
    if (it != indexes.end()) {
        return it->second->find(value);
    }
    return {};
}

} // namespace Storage
} // namespace TissDB
