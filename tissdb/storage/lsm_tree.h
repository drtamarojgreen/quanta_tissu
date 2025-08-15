#pragma once

#include <cstddef>
#include <string>
#include <memory>
#include <map>
#include <optional>
#include <vector>

#include "collection.h"
#include "transaction_manager.h"

namespace TissDB {
namespace Storage {

class LSMTree {
public:
    explicit LSMTree(const std::string& data_dir);
    ~LSMTree();

    // Collection management
    void create_collection(const std::string& name);
    void delete_collection(const std::string& name);
    std::vector<std::string> list_collections() const;

    // Document operations (delegated to specific collection)
    void put(const std::string& collection_name, const std::string& key, const Document& doc, int transaction_id = -1);
    std::optional<Document> get(const std::string& collection_name, const std::string& key);
    void del(const std::string& collection_name, const std::string& key, int transaction_id = -1);
    std::vector<Document> scan(const std::string& collection_name);
    void create_index(const std::string& collection_name, const std::vector<std::string>& field_names);
    bool has_index(const std::string& collection_name, const std::vector<std::string>& field_names) const;
    std::vector<Document> get_all(const std::string& collection_name);

    std::vector<std::string> find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value);

    // Transaction management
    int begin_transaction();
    void commit_transaction(int transaction_id);
    void rollback_transaction(int transaction_id);

    // Helper to get a collection, throws if not found
    Collection& get_collection(const std::string& name);

private:
    std::string data_directory_;
    std::map<std::string, std::unique_ptr<Collection>> collections_;
    TissDB::Transactions::TransactionManager transaction_manager_;

    void shutdown();
};

} // namespace Storage
} // namespace TissDB
