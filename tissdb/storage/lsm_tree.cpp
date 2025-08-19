#include "lsm_tree.h"
#include "../common/log.h"
#include <stdexcept>

namespace TissDB {
namespace Storage {

LSMTree::LSMTree() : path_(""), transaction_manager_(*this) {}

LSMTree::LSMTree(const std::string& path) : path_(path), transaction_manager_(*this) {}

LSMTree::~LSMTree() {}

void LSMTree::create_collection(const std::string& name, const TissDB::Schema& schema) {
    if (collections_.count(name)) {
        LOG_ERROR("Attempted to create collection that already exists: " + name);
        throw std::runtime_error("Collection already exists: " + name);
    }
    LOG_INFO("Creating collection: " + name);
    auto collection = std::make_unique<Collection>(name, this);
    collection->set_schema(schema);
    collections_[name] = std::move(collection);
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
    if (tid != -1) {
        transaction_manager_.add_put_operation(tid, collection_name, key, doc);
    } else {
        try {
            Collection& collection = get_collection(collection_name);
            collection.put(key, doc);
        } catch (const std::runtime_error& e) {
            // Collection not found, ignore
        }
    }
}

std::optional<std::shared_ptr<Document>> LSMTree::get(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
    if (tid != -1) {
        const auto* transaction = transaction_manager_.get_transaction(tid);
        if (transaction) {
            // Iterate backwards to find the most recent operation for this key
            const auto& operations = transaction->get_operations();
            for (auto it = operations.rbegin(); it != operations.rend(); ++it) {
                if (it->collection_name == collection_name && it->key == key) {
                    if (it->type == Transactions::OperationType::PUT) {
                        return std::make_shared<Document>(it->doc);
                    }
                    if (it->type == Transactions::OperationType::DELETE) {
                        return nullptr; // Tombstone
                    }
                }
            }
        }
    }

    // If not in transaction or transaction doesn't exist, get from main storage
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
    if (tid != -1) {
        transaction_manager_.add_delete_operation(tid, collection_name, key);
    } else {
        try {
            Collection& collection = get_collection(collection_name);
            collection.del(key);
        } catch (const std::runtime_error& e) {
            // Collection not found, ignore
        }
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

const std::string& LSMTree::get_path() const {
    return path_;
}

const Collection& LSMTree::get_collection(const std::string& name) const {
    auto it = collections_.find(name);
    if (it == collections_.end()) {
        throw std::runtime_error("Collection not found: " + name);
    }
    return *it->second;
}

void LSMTree::create_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
    try {
        Collection& collection = get_collection(collection_name);
        collection.create_index(field_names);
    } catch (const std::runtime_error& e) {
        // Collection not found, ignore or log
    }
}

std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) {
    try {
        const Collection& collection = get_collection(collection_name);
        return collection.find_by_index(field_name, value);
    } catch (const std::runtime_error& e) {
        return {};
    }
}

std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values) {
    try {
        const Collection& collection = get_collection(collection_name);
        return collection.find_by_index(field_names, values);
    } catch (const std::runtime_error& e) {
        return {};
    }
}

Transactions::TransactionID LSMTree::begin_transaction() {
    return transaction_manager_.begin_transaction();
}

void LSMTree::commit_transaction(Transactions::TransactionID transaction_id) {
    transaction_manager_.commit_transaction(transaction_id);
}

void LSMTree::rollback_transaction(Transactions::TransactionID transaction_id) {
    transaction_manager_.rollback_transaction(transaction_id);
}

bool LSMTree::has_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
    try {
        const Collection& collection = get_collection(collection_name);
        return collection.has_index(field_names);
    } catch (const std::runtime_error& e) {
        return false;
    }
}

std::vector<std::vector<std::string>> LSMTree::get_available_indexes(const std::string& collection_name) const {
    try {
        const Collection& collection = get_collection(collection_name);
        return collection.get_available_indexes();
    } catch (const std::runtime_error& e) {
        return {};
    }
}

void LSMTree::shutdown() {
    for (auto const& [name, collection] : collections_) {
        collection->shutdown();
    }
}

} // namespace Storage
} // namespace TissDB