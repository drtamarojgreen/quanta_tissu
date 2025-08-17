#include "lsm_tree.h"
#include <stdexcept>

namespace TissDB {
namespace Storage {

LSMTree::LSMTree() {}

LSMTree::~LSMTree() {}

void LSMTree::create_collection(const std::string& name, const TissDB::Schema& schema) {
    if (collections_.count(name)) {
        throw std::runtime_error("Collection already exists: " + name);
    }
    collections_[name] = std::make_unique<Collection>(schema); // Assuming Collection constructor takes schema
}

void LSMTree::delete_collection(const std::string& name) {
    if (!collections_.count(name)) {
        throw std::runtime_error("Collection does not exist: " + name);
    }
    collections_.erase(name);
}

std::vector<std::string> LSMTree::list_collections() const {
    std::vector<std::string> names;
    for (const auto& pair : collections_) {
        names.push_back(pair.first);
    }
    return names;
}

void LSMTree::put(const std::string& collection_name, const std::string& key, const Document& doc, Transactions::TransactionID tid) {
    Collection* collection = get_collection_ptr(collection_name);
    if (collection) {
        collection->put(key, doc, tid);
    }
}

std::optional<std::shared_ptr<Document>> LSMTree::get(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
    Collection* collection = get_collection_ptr(collection_name);
    if (collection) {
        return collection->get(key, tid);
    }
    return std::nullopt;
}

void LSMTree::del(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
    Collection* collection = get_collection_ptr(collection_name);
    if (collection) {
        collection->del(key, tid);
    }
}

std::vector<Document> LSMTree::scan(const std::string& collection_name) {
    Collection* collection = get_collection_ptr(collection_name);
    if (collection) {
        return collection->scan();
    }
    return {}; // Return empty vector if collection not found
}

Collection* LSMTree::get_collection_ptr(const std::string& name) {
    auto it = collections_.find(name);
    if (it == collections_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Collection& LSMTree::get_collection(const std::string& name) {
    auto it = collections_.find(name);
    if (it == collections_.end()) {
        throw std::runtime_error("Collection not found: " + name);
    }
    return *it->second;
}

const Collection& LSMTree::get_collection(const std::string& name) const {
    auto it = collections_.find(name);
    if (it == collections_.end()) {
        throw std::runtime_error("Collection not found: " + name);
    }
    return *it->second;
}

void LSMTree::create_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
    // Placeholder: Implement index creation logic
    throw std::runtime_error("create_index not yet implemented");
}

std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) {
    // Placeholder: Implement single-field index lookup
    throw std::runtime_runtime_error("find_by_index (single field) not yet implemented");
}

std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values) {
    // Placeholder: Implement multi-field index lookup
    throw std::runtime_error("find_by_index (multi-field) not yet implemented");
}

Transactions::TransactionID LSMTree::begin_transaction() {
    // Placeholder: Implement transaction begin logic
    throw std::runtime_error("begin_transaction not yet implemented");
}

void LSMTree::commit_transaction(Transactions::TransactionID transaction_id) {
    // Placeholder: Implement transaction commit logic
    throw std::runtime_error("commit_transaction not yet implemented");
}

void LSMTree::rollback_transaction(Transactions::TransactionID transaction_id) {
    // Placeholder: Implement transaction rollback logic
    throw std::runtime_error("rollback_transaction not yet implemented");
}

bool LSMTree::has_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
    // Placeholder: Implement index check logic
    return false;
}

std::vector<std::vector<std::string>> LSMTree::get_available_indexes(const std::string& collection_name) const {
    // Placeholder: Implement available indexes logic
    return {};
}

void LSMTree::shutdown() {
    // Placeholder: Implement shutdown logic
}

} // namespace Storage
} // namespace TissDB
