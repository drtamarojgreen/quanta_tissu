#include "lsm_tree.h"
#include "wal.h"
#include "../common/log.h"
#include <stdexcept>
#include <filesystem>

namespace TissDB {
namespace Storage {

namespace fs = std::filesystem;

LSMTree::LSMTree() : path_(""), wal_(nullptr) {
    // This default constructor should probably not be used in production.
    // A database needs a path.
}

LSMTree::LSMTree(const std::string& path) : path_(path) {
    std::string wal_path = fs::path(path) / "wal.log";
    wal_ = std::make_unique<WriteAheadLog>(wal_path);
    recover_from_wal();
}

LSMTree::~LSMTree() {
    // The unique_ptr for wal_ will handle its destruction.
}

void LSMTree::recover_from_wal() {
    LOG_INFO("LSMTree: Starting recovery for database at " + path_);
    auto entries = wal_->recover();
    for (const auto& entry : entries) {
        if (entry.type == LogEntryType::CREATE_COLLECTION) {
            if (!collections_.count(entry.collection_name)) {
                collections_[entry.collection_name] = std::make_unique<Collection>();
            }
        } else if (entry.type == LogEntryType::DELETE_COLLECTION) {
            if (collections_.count(entry.collection_name)) {
                collections_.erase(entry.collection_name);
            }
        } else if (entry.type == LogEntryType::PUT) {
            if (!collections_.count(entry.collection_name)) {
                collections_[entry.collection_name] = std::make_unique<Collection>();
            }
            Collection& collection = get_collection(entry.collection_name);
            collection.put(entry.document_id, entry.doc);
        } else if (entry.type == LogEntryType::DELETE) {
            if (collections_.count(entry.collection_name)) {
                Collection& collection = get_collection(entry.collection_name);
                collection.del(entry.document_id);
            }
        }
    }
    LOG_INFO("LSMTree: Recovery complete for database at " + path_);
}


void LSMTree::create_collection(const std::string& name, const TissDB::Schema& schema) {
    if (collections_.count(name)) {
        throw std::runtime_error("Collection already exists: " + name);
    }

    LogEntry entry;
    entry.type = LogEntryType::CREATE_COLLECTION;
    entry.collection_name = name;
    wal_->append(entry);

    LOG_INFO("Creating collection: " + name + " in database: " + path_);
    collections_[name] = std::make_unique<Collection>();
}

void LSMTree::delete_collection(const std::string& name) {
    if (!collections_.count(name)) {
        throw std::runtime_error("Collection does not exist: " + name);
    }

    LogEntry entry;
    entry.type = LogEntryType::DELETE_COLLECTION;
    entry.collection_name = name;
    wal_->append(entry);

    LOG_INFO("Deleting collection: " + name + " from database: " + path_);
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
    if (!collections_.count(collection_name)) {
        create_collection(collection_name, TissDB::Schema{});
    }

    LogEntry entry;
    entry.type = LogEntryType::PUT;
    entry.transaction_id = tid;
    entry.collection_name = collection_name;
    entry.document_id = key;
    entry.doc = doc;
    wal_->append(entry);

    Collection& collection = get_collection(collection_name);
    collection.put(key, doc);
}

std::optional<std::shared_ptr<Document>> LSMTree::get(const std::string& collection_name, const std::string& key, Transactions::TransactionID /*tid*/) {
    if (!collections_.count(collection_name)) {
        return std::nullopt;
    }
    Collection& collection = get_collection(collection_name);
    return collection.get(key);
}

std::vector<Document> LSMTree::get_many(const std::string& collection_name, const std::vector<std::string>& keys) {
    std::vector<Document> result_docs;
    if (!collections_.count(collection_name)) {
        return result_docs;
    }
    Collection& collection = get_collection(collection_name);
    for (const auto& key : keys) {
        auto doc_opt = collection.get(key);
        if (doc_opt) {
            result_docs.push_back(**doc_opt);
        }
    }
    return result_docs;
}

void LSMTree::del(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
    if (!collections_.count(collection_name)) {
        return; // Can't delete from a non-existent collection
    }

    LogEntry entry;
    entry.type = LogEntryType::DELETE;
    entry.transaction_id = tid;
    entry.collection_name = collection_name;
    entry.document_id = key;
    wal_->append(entry);

    Collection& collection = get_collection(collection_name);
    collection.del(key);
}

std::vector<Document> LSMTree::scan(const std::string& collection_name) {
    if (!collections_.count(collection_name)) {
        return {};
    }
    Collection& collection = get_collection(collection_name);
    return collection.scan();
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

// --- Placeholder Implementations ---

void LSMTree::create_index(const std::string&, const std::vector<std::string>&) {
    throw std::runtime_error("create_index not yet implemented");
}

std::vector<std::string> LSMTree::find_by_index(const std::string&, const std::string&, const std::string&) {
    throw std::runtime_error("find_by_index (single field) not yet implemented");
}

std::vector<std::string> LSMTree::find_by_index(const std::string&, const std::vector<std::string>&, const std::vector<std::string>&) {
    throw std::runtime_error("find_by_index (multi-field) not yet implemented");
}

Transactions::TransactionID LSMTree::begin_transaction() {
    throw std::runtime_error("begin_transaction not yet implemented");
}

void LSMTree::commit_transaction(Transactions::TransactionID) {
    throw std::runtime_error("commit_transaction not yet implemented");
}

void LSMTree::rollback_transaction(Transactions::TransactionID) {
    throw std::runtime_error("rollback_transaction not yet implemented");
}

bool LSMTree::has_index(const std::string&, const std::vector<std::string>&) {
    return false;
}

std::vector<std::vector<std::string>> LSMTree::get_available_indexes(const std::string&) const {
    return {};
}

void LSMTree::shutdown() {
    if (wal_) {
        wal_->shutdown();
    }
}

} // namespace Storage
} // namespace TissDB