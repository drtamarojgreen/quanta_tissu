#pragma once

#include <cstddef>
#include <string>
#include <map>
#include <optional>
#include <memory>
#include <vector>

#include "../common/document.h"
#include "../common/schema.h"
#include "indexer.h"

namespace TissDB {
namespace Storage {

class LSMTree; // Forward declaration

// A Collection is an in-memory sorted data structure that holds all documents for a single collection.
// It is managed by the Database class.
class Collection {
public:
    Collection(LSMTree* parent_db);
    Collection(const std::string& path, LSMTree* parent_db);

    // Inserts or updates a document in the collection.
    void put(const std::string& key, const Document& doc);

    // Marks a document as deleted by writing a "tombstone".
    void del(const std::string& key);

    // Retrieves a document from the collection.
    // Returns a pointer to the document if found.
    // Returns `std::nullopt` if the key is not found.
    // Returns a `nullptr` inside the optional if the key was deleted (tombstone).
    std::optional<std::shared_ptr<Document>> get(const std::string& key);

    // Returns all key-value pairs, sorted by key.
    // This is used when flushing the collection to an SSTable on disk.
    const std::map<std::string, std::shared_ptr<Document>>& get_all() const;

    // Clears all data from the collection.
    void clear();

    // Returns the approximate size of the collection in bytes.
    size_t approximate_size() const;

    // Scans all documents in the collection.
    std::vector<Document> scan() const;

    void set_schema(const TissDB::Schema& schema);
    void create_index(const std::vector<std::string>& field_names);
    void shutdown();

private:
    // We use a sorted map to store documents in memory. The key is the document ID.
    // A shared_ptr to a Document allows us to distinguish between:
    // 1. Key not present -> map::find() returns end()
    // 2. Key present with a document -> non-null shared_ptr
    // 3. Key present but deleted -> null shared_ptr (tombstone)
    std::map<std::string, std::shared_ptr<Document>> data;
    size_t estimated_size;
    TissDB::Schema schema_;
    LSMTree* parent_db_; // Pointer to the parent database
    std::unique_ptr<Indexer> indexer_;
};

} // namespace Storage
} // namespace TissDB