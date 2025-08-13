#pragma once

#include "collection.h"
#include <string>
#include <memory>
#include <map>
#include <optional>

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
    void put(const std::string& collection_name, const std::string& key, const Document& doc);
    std::optional<Document> get(const std::string& collection_name, const std::string& key);
    void del(const std::string& collection_name, const std::string& key);
    std::vector<Document> scan(const std::string& collection_name);
    void create_index(const std::string& collection_name, const std::string& field_name);

    std::vector<std::string> find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value);

private:
    std::string data_directory_;
    std::map<std::string, std::unique_ptr<Collection>> collections_;

    // Helper to get a collection, throws if not found
    Collection& get_collection(const std::string& name);
};

} // namespace Storage
} // namespace TissDB