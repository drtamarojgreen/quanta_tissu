#pragma once

#include <string>
#include <memory>
#include <map>
#include <optional>
#include <vector>

#include "collection.h"
#include "transaction_manager.h"
#include "../common/schema.h"
#include "wal.h"

namespace TissDB {
namespace Storage {

// LSMTree acts as the main database interface, managing all collections.
// In this simplified, in-memory version, it holds collections in a map.
class LSMTree {
public:
    LSMTree(); // Simplified constructor
    LSMTree(const std::string& path);
    ~LSMTree();

    WriteAheadLog& get_wal() { return *wal_; }

    // Recovery
    void recover();

    // Collection management
    virtual void create_collection(const std::string& name, const TissDB::Schema& schema, bool is_recovery = false);
    virtual void delete_collection(const std::string& name);
    virtual std::vector<std::string> list_collections() const;

    // Document operations (delegated to specific collection)
    virtual void put(const std::string& collection_name, const std::string& key, const Document& doc, Transactions::TransactionID tid = -1, bool is_recovery = false);
    virtual std::optional<std::shared_ptr<Document>> get(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid = -1);
    virtual std::vector<Document> get_many(const std::string& collection_name, const std::vector<std::string>& keys);
    virtual void del(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid = -1, bool is_recovery = false);
    virtual std::vector<Document> scan(const std::string& collection_name);
    virtual void create_index(const std::string& collection_name, const std::vector<std::string>& field_names);

    virtual std::vector<std::string> find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value);
    std::vector<std::string> find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values);

    // Transaction management
    Transactions::TransactionID begin_transaction();
    bool commit_transaction(Transactions::TransactionID transaction_id);
    bool rollback_transaction(Transactions::TransactionID transaction_id);

    // Helper to get a collection, throws if not found
    Collection& get_collection(const std::string& name);
    const Collection& get_collection(const std::string& name) const;

    bool has_index(const std::string& collection_name, const std::vector<std::string>& field_names);
    std::vector<std::vector<std::string>> get_available_indexes(const std::string& collection_name) const;
    void shutdown();

private:
    std::map<std::string, std::unique_ptr<Collection>> collections_;
    std::string path_;
    std::unique_ptr<WriteAheadLog> wal_;
    Transactions::TransactionManager transaction_manager_;
};

} // namespace Storage
} // namespace TissDB
