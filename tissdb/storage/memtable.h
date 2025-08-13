#pragma once

#include "../common/document.h"
#include <string>
#include <map>
#include <optional>
#include <memory>

namespace TissDB {
namespace Storage {

// The Memtable is an in-memory sorted data structure that buffers recent writes.
// When the memtable reaches a certain size, it is flushed to a file on disk (SSTable).
class Memtable {
public:
    Memtable();

    // Inserts or updates a document in the memtable.
    void put(const std::string& key, const Document& doc);

    // Marks a document as deleted by writing a "tombstone".
    void del(const std::string& key);

    // Retrieves a document from the memtable.
    // Returns a pointer to the document if found.
    // Returns `std::nullopt` if the key is not found.
    // Returns a `nullptr` inside the optional if the key was deleted (tombstone).
    std::optional<std::shared_ptr<Document>> get(const std::string& key);

    // Returns all key-value pairs, sorted by key.
    // This is used when flushing the memtable to an SSTable on disk.
    const std::map<std::string, std::shared_ptr<Document>>& get_all() const;

    // Clears all data from the memtable.
    void clear();

    // Returns the approximate size of the memtable in bytes.
    size_t approximate_size() const;

    // Scans all documents in the memtable.
    std::vector<Document> scan() const;

private:
    // We use a sorted map to store documents in memory. The key is the document ID.
    // A shared_ptr to a Document allows us to distinguish between:
    // 1. Key not present -> map::find() returns end()
    // 2. Key present with a document -> non-null shared_ptr
    // 3. Key present but deleted -> null shared_ptr (tombstone)
    std::map<std::string, std::shared_ptr<Document>> data;
    size_t estimated_size;
};

} // namespace Storage
} // namespace TissDB
