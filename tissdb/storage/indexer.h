#pragma once

#include "../common/document.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace TissDB {
namespace Storage {

// Represents a B-Tree index for a single field.
// This is a placeholder; a real implementation would use a persistent B-Tree data structure.
// It maps a field's value to a list of document IDs that contain that value.
class BTreeIndex {
public:
    // Adds a document ID to the index for a given value.
    void add(const std::string& value, const std::string& document_id);

    // Removes a document ID from the index for a given value.
    void remove(const std::string& value, const std::string& document_id);

    // Finds all document IDs associated with a given value.
    std::vector<std::string> find(const std::string& value) const;

private:
    // For this placeholder, we use a simple std::map to simulate the B-Tree's functionality.
    std::map<std::string, std::vector<std::string>> index_data;
};

// The Indexer manages all the indexes for a single collection.
class Indexer {
public:
    Indexer();

    // Creates a new (empty) index on a given field.
    void create_index(const std::string& field_name);

    // Checks if an index exists for a given field.
    bool has_index(const std::string& field_name) const;

    // Updates all relevant indexes for a document when it is added or updated.
    void update_indexes(const std::string& document_id, const Document& doc);

    // Removes a document from all relevant indexes when it is deleted.
    void remove_from_indexes(const std::string& document_id, const Document& doc);

    // Uses an index to find document IDs matching a specific field value.
    // Returns an empty vector if no index exists for the field.
    std::vector<std::string> find_by_index(const std::string& field_name, const std::string& value) const;

private:
    // A map from a field name (e.g., "brand") to its corresponding B-Tree index.
    std::map<std::string, std::unique_ptr<BTreeIndex>> indexes;
};

} // namespace Storage
} // namespace TissDB
