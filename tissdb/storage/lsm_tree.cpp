#include "lsm_tree.h"
#include "../common/log.h"
#include "../common/serialization.h"
#include "../crypto/kms.h"
#include <stdexcept>
#include <filesystem>
#include <set>

namespace TissDB {
namespace Storage {

namespace {
// This is a placeholder for a proper dependency injection mechanism.
TissDB::Crypto::KeyManagementSystem& get_kms_instance() {
    // This master key should be loaded from a secure configuration or vault, not hardcoded.
    static TissDB::Crypto::Key master_key = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    };
    static TissDB::Crypto::KeyManagementSystem instance(master_key);
    return instance;
}
} // anonymous namespace

void replay_log_entry(LSMTree* tree, const LogEntry& entry);

LSMTree::LSMTree() : path_(""), transaction_manager_(*this) {}

LSMTree::LSMTree(const std::string& path) : path_(path), transaction_manager_(*this) {
    std::filesystem::path db_path(path_);
    if (!std::filesystem::exists(db_path)) {
        std::filesystem::create_directories(db_path);
    }

    std::string wal_path = (db_path / "wal.log").string();
    wal_ = std::make_unique<WriteAheadLog>(wal_path);

    LOG_INFO("Database opened at: " + path_ + ". Starting recovery.");
    recover();
    LOG_INFO("Recovery complete.");
    LOG_INFO("Loading collections and indexes...");
    load_collections();
    LOG_INFO("Collection loading complete.");
}

void LSMTree::recover() {
    auto log_entries = wal_->recover();
    std::set<Transactions::TransactionID> aborted_tids;

    // First pass: find aborted transactions
    for (const auto& entry : log_entries) {
        if (entry.type == LogEntryType::TXN_ABORT) {
            aborted_tids.insert(entry.transaction_id);
        }
    }

    // Second pass: replay committed entries
    for (const auto& entry : log_entries) {
        switch (entry.type) {
            case LogEntryType::PUT:
                put(entry.collection_name, entry.document_id, entry.doc, -1, true);
                break;
            case LogEntryType::DELETE:
                del(entry.collection_name, entry.document_id, -1, true);
                break;
            case LogEntryType::CREATE_COLLECTION:
                if (!collections_.count(entry.collection_name)) {
                    create_collection(entry.collection_name, {}, true);
                } else {
                    LOG_WARNING("Recovery: Attempted to re-create collection '" + entry.collection_name + "' which already exists. Skipping.");
                }
                break;
            case LogEntryType::TXN_COMMIT:
                if (aborted_tids.find(entry.transaction_id) == aborted_tids.end()) {
                    for (const auto& op : entry.operations) {
                        if (op.type == Transactions::OperationType::PUT) {
                            put(op.collection_name, op.key, op.doc, -1, true);
                        } else if (op.type == Transactions::OperationType::DELETE) {
                            del(op.collection_name, op.key, -1, true);
                        }
                    }
                }
                break;
            default:
                // Other types like ABORT, DELETE_COLLECTION etc. are ignored during replay
                break;
        }
    }
}

LSMTree::~LSMTree() {
    if (wal_) {
        wal_->shutdown();
    }
}

void LSMTree::create_collection(const std::string& name, const TissDB::Schema& schema, bool is_recovery) {
    if (collections_.count(name)) {
        LOG_ERROR("Attempted to create collection that already exists: " + name);
        throw std::runtime_error("Collection already exists: " + name);
    }

    if (!is_recovery) {
        LogEntry entry;
        entry.type = LogEntryType::CREATE_COLLECTION;
        entry.collection_name = name;
        // Note: Schema is not persisted in the WAL.
        wal_->append(entry);
    }

    LOG_INFO("Creating collection: " + name);
    std::string collection_path = (std::filesystem::path(path_) / name).string();
    if (!std::filesystem::exists(collection_path)) {
        std::filesystem::create_directories(collection_path);
    }
    auto collection = std::make_unique<Collection>(this, collection_path);
    collection->set_schema(schema);
    collections_[name] = std::move(collection);
}

void LSMTree::delete_collection(const std::string& name) {
    if (!collections_.count(name)) {
        LOG_ERROR("Attempted to delete collection that does not exist: " + name);
        throw std::runtime_error("Collection does not exist: " + name);
    }

    // --- Cryptographic Shredding ---
    // By deleting the Data Encryption Key, the data in the SSTables becomes
    // permanently unrecoverable, even if the files were to be recovered later.
    LOG_INFO("Shredding encryption key for collection: " + name);
    get_kms_instance().delete_dek(name);
    // --- End Cryptographic Shredding ---

    LogEntry entry;
    entry.type = LogEntryType::DELETE_COLLECTION;
    entry.collection_name = name;
    wal_->append(entry);

    LOG_INFO("Deleting collection: " + name);
    collections_.erase(name);

    // Also delete the directory from disk
    try {
        std::string collection_path = (std::filesystem::path(path_) / name).string();
        if (std::filesystem::exists(collection_path)) {
            std::filesystem::remove_all(collection_path);
            LOG_INFO("Removed collection data directory: " + collection_path);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Error deleting collection directory: " + std::string(e.what()));
        // Don't re-throw, as the collection is already gone from memory and WAL.
    }
}

std::vector<std::string> LSMTree::list_collections() const {
    std::vector<std::string> names;
    for (const auto& pair : collections_) {
        names.push_back(pair.first);
    }
    return names;
}

void LSMTree::put(const std::string& collection_name, const std::string& key, const Document& doc, Transactions::TransactionID tid, bool is_recovery) {
    if (tid != -1) {
        transaction_manager_.add_put_operation(tid, collection_name, key, doc);
    } else {
        if (!is_recovery) {
            LogEntry entry;
            entry.type = LogEntryType::PUT;
            entry.collection_name = collection_name;
            entry.document_id = key;
            entry.doc = doc;
            wal_->append(entry);
        }

        try {
            Collection& collection = get_collection(collection_name);
            collection.put(key, doc);
        } catch (const std::runtime_error& e) {
            // Collection not found, ignore.
            // The WAL entry is still written, which is acceptable.
        }

        Collection& collection = get_collection(collection_name);
        collection.put(key, doc);
    }
}

std::optional<std::shared_ptr<Document>> LSMTree::get(const std::string& collection_name, const std::string& key, Transactions::TransactionID /*tid*/) {
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
            if (doc_opt && *doc_opt) {
                result_docs.push_back(**doc_opt);
            }
        }
    } catch (const std::runtime_error& e) {
        // Collection not found, return empty vector
    }
    return result_docs;
}


bool LSMTree::del(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid, bool is_recovery) {
    if (tid != -1) {
        transaction_manager_.add_delete_operation(tid, collection_name, key);
        return true; // Assume success for transactional deletes for now
    } else {
        if (!is_recovery) {
            LogEntry entry;
            entry.type = LogEntryType::DELETE;
            entry.collection_name = collection_name;
            entry.document_id = key;
            wal_->append(entry);
        }

        try {
            Collection& collection = get_collection(collection_name);
            return collection.del(key);
        } catch (const std::runtime_error& e) {
            // Collection not found
            return false;
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

const Collection& LSMTree::get_collection(const std::string& name) const {
    auto it = collections_.find(name);
    if (it == collections_.end()) {
        throw std::runtime_error("Collection not found: " + name);
    }
    return *it->second;
}

void LSMTree::create_index(const std::string& collection_name, const std::vector<std::string>& field_names, bool is_unique) {
    try {
        Collection& collection = get_collection(collection_name);
        collection.create_index(field_names, is_unique);
    } catch (const std::runtime_error& e) {
        LOG_ERROR("Error creating index: " + std::string(e.what()));
        throw;
    }
}

std::vector<std::string> LSMTree::find_by_index(const std::string& /*collection_name*/, const std::string& /*field_name*/, const std::string& /*value*/) {
    // Placeholder: Implement single-field index lookup
    throw std::runtime_error("find_by_index (single field) not yet implemented");
}

std::vector<std::string> LSMTree::find_by_index(const std::string& /*collection_name*/, const std::vector<std::string>& /*field_names*/, const std::vector<std::string>& /*values*/) {
    // Placeholder: Implement multi-field index lookup
    throw std::runtime_error("find_by_index (multi-field) not yet implemented");
}

Transactions::TransactionID LSMTree::begin_transaction() {
    return transaction_manager_.begin_transaction();
}

bool LSMTree::commit_transaction(Transactions::TransactionID transaction_id) {
    return transaction_manager_.commit_transaction(transaction_id);
}

bool LSMTree::rollback_transaction(Transactions::TransactionID transaction_id) {
    return transaction_manager_.rollback_transaction(transaction_id);
}

bool LSMTree::has_index(const std::string& /*collection_name*/, const std::vector<std::string>& /*field_names*/) {
    // Placeholder: Implement index check logic
    return false;
}

std::vector<std::vector<std::string>> LSMTree::get_available_indexes(const std::string& /*collection_name*/) const {
    // Placeholder: Implement available indexes logic
    return {};
}

void LSMTree::shutdown() {
    LOG_INFO("Shutting down database at: " + path_);
    save_collections();
    if (wal_) {
        wal_->shutdown();
    }
    LOG_INFO("Database shutdown complete.");
}

void LSMTree::load_collections() {
    namespace fs = std::filesystem;
    if (!fs::exists(path_) || !fs::is_directory(path_)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(path_)) {
        if (entry.is_directory()) {
            std::string collection_name = entry.path().filename().string();
            if (collections_.find(collection_name) == collections_.end()) {
                LOG_INFO("Discovered and loading collection: " + collection_name);
                std::string collection_path = entry.path().string();
                // Use the constructor that takes a path
                collections_[collection_name] = std::make_unique<Collection>(collection_path, this);
            }
        }
    }
}

void LSMTree::save_collections() {
    LOG_INFO("Saving all collection indexes...");
    for (auto const& [name, collection] : collections_) {
        collection->save_indexes();
    }
    LOG_INFO("Finished saving all collection indexes.");
}

void replay_log_entry(LSMTree* tree, const LogEntry& entry) {
    switch (entry.type) {
        case LogEntryType::CREATE_COLLECTION:
            try {
                // NOTE: The schema is not persisted in the WAL. Using an empty schema for recovery.
                tree->create_collection(entry.collection_name, TissDB::Schema({}));
            } catch (const std::runtime_error& e) {
                // It's possible the collection already exists if the log has duplicate entries.
                LOG_WARNING("During WAL replay, could not create collection '" + entry.collection_name + "': " + e.what());
            }
            break;
        case LogEntryType::DELETE_COLLECTION:
            try {
                tree->delete_collection(entry.collection_name);
            } catch (const std::runtime_error& e) {
                LOG_WARNING("During WAL replay, could not delete collection '" + entry.collection_name + "': " + e.what());
            }
            break;
        case LogEntryType::PUT:
            tree->put(entry.collection_name, entry.document_id, entry.doc);
            break;
        case LogEntryType::DELETE:
            tree->del(entry.collection_name, entry.document_id);
            break;
        case LogEntryType::TXN_COMMIT:
        case LogEntryType::TXN_ABORT:
            // Transaction replay logic is not yet implemented.
            break;
    }
}

} // namespace Storage
} // namespace TissDB