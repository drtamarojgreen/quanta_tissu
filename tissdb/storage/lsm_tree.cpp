#include "lsm_tree.h"
#include "../common/log.h"
#include <stdexcept>

namespace TissDB {
namespace Storage {

LSMTree::LSMTree() : path_("") {}

LSMTree::LSMTree(const std::string& path) : path_(path) {}

LSMTree::~LSMTree() {}

void LSMTree::create_collection(const std::string& name, const TissDB::Schema& schema) {
    if (collections_.count(name)) {
        LOG_ERROR("Attempted to create collection that already exists: " + name);
        throw std::runtime_error("Collection already exists: " + name);
    }
    LOG_INFO("Creating collection: " + name);
    collections_[name] = std::make_unique<Collection>(); // Assuming Collection constructor takes schema
}

void LSMTree::delete_collection(const std::string& name) {
    if (!collections_.count(name)) {
        LOG_ERROR("Attempted to delete collection that does not exist: " + name);
        throw std::runtime_error("Collection does not exist: " + name);
    }
    LOG_INFO("Deleting collection: " + name);
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
    try {
        Collection& collection = get_collection(collection_name);
        collection.put(key, doc);
    } catch (const std::runtime_error& e) {
        // Collection not found, ignore
    }
}

std::optional<std::shared_ptr<Document>> LSMTree::get(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
    try {
        Collection& collection = get_collection(collection_name);
        return collection.get(key);
    } catch (const std::runtime_error& e) {
        // Collection not found
        return std::nullopt;
    }
}

std::vector<Document> LSMTree::get_many(const std::string& collection_name, const std::vector<std::string>& keys) {
    std::vector<Document> result_docs;
    try {
        Collection& collection = get_collection(collection_name);
        for (const auto& key : keys) {
            auto doc_opt = collection.get(key);
            if (doc_opt) {
                result_docs.push_back(**doc_opt);
            }
        }
    } catch (const std::runtime_error& e) {
        // Collection not found, return empty vector
    }
    return result_docs;
}


void LSMTree::del(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
    try {
        Collection& collection = get_collection(collection_name);
        collection.del(key);
    } catch (const std::runtime_error& e) {
        // Collection not found, ignore
    }
}

std::vector<Document> LSMTree::scan(const std::string& collection_name) {
    try {
        Collection& collection = get_collection(collection_name);
        return collection.scan();
    } catch (const std::runtime_error& e) {
        // Collection not found
        return {}; // Return empty vector if collection not found
    }
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
    throw std::runtime_error("find_by_index (single field) not yet implemented");
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