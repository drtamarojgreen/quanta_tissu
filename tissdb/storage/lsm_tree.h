#pragma once

#include <cstddef>
#include <string>
#include <memory>
#include <map>
#include <optional>
#include <vector>

#include "collection.h"
#include "transaction_manager.h"
#include "../common/schema.h"

namespace TissDB {
namespace Storage {

class LSMTree {
public:
    explicit LSMTree(const std::string& data_dir);
    ~LSMTree();

    // Collection management
    void create_collection(const std::string& name, const TissDB::Schema& schema);
    void delete_collection(const std::string& name);
    std::vector<std::string> list_collections() const;

    // Document operations (delegated to specific collection)
    void put(const std::string& collection_name, const std::string& key, const Document& doc, Transactions::TransactionID tid = -1);
    std::optional<Document> get(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid = -1);
    void del(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid = -1);
    std::vector<Document> scan(const std::string& collection_name);
    void create_index(const std::string& collection_name, const std::vector<std::string>& field_names);

    std::vector<std::string> find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value);
    std::vector<std::string> find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values);

    // Transaction management
    Transactions::TransactionID begin_transaction();
    void commit_transaction(Transactions::TransactionID transaction_id);
    void rollback_transaction(Transactions::TransactionID transaction_id);

    // Helper to get a collection, throws if not found
    Collection& get_collection(const std::string& name);
    const Collection& get_collection(const std::string& name) const;

    bool has_index(const std::string& collection_name, const std::vector<std::string>& field_names);
    std::vector<std::vector<std::string>> get_available_indexes(const std::string& collection_name) const;
    void shutdown();

private:
    std::string data_directory_;
    std::map<std::string, std::unique_ptr<Collection>> collections_;
    Transactions::TransactionManager transaction_manager_;
};

} // namespace Storage
} // namespace TissDB
