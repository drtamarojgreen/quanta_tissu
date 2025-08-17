#pragma once

#include <string>
#include <memory>
#include <map>
#include <optional>
#include <vector>

#include "collection.h"

namespace TissDB {
namespace Storage {

// LSMTree acts as the main database interface, managing all collections.
// In this simplified, in-memory version, it holds collections in a map.
class LSMTree {
public:
    LSMTree(); // Simplified constructor
    ~LSMTree();

    // Collection management
    bool create_collection(const std::string& name);
    bool drop_collection(const std::string& name);
    std::vector<std::string> list_collections() const;

    // Document operations (delegated to specific collection)
    void put(const std::string& collection_name, const std::string& key, const Document& doc);
    std::optional<std::shared_ptr<Document>> get(const std::string& collection_name, const std::string& key);
    void del(const std::string& collection_name, const std::string& key);
    std::vector<Document> scan(const std::string& collection_name);

    // Helper to get a collection
    Collection* get_collection(const std::string& name);

private:
    std::map<std::string, std::unique_ptr<Collection>> collections_;
};

} // namespace Storage
} // namespace TissDB
