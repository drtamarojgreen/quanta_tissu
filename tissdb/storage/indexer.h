#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "native_b_tree.h"

#include "../common/document.h"

namespace TissDB {
namespace Storage {

// The Indexer class manages all B+ tree indexes for the database.
class Indexer {
public:
    Indexer() = default;

    void create_index(const std::vector<std::string>& field_names);
    bool has_index(const std::vector<std::string>& field_names) const;
    void update_indexes(const std::string& document_id, const Document& doc);
    void remove_from_indexes(const std::string& document_id, const Document& doc);
    std::vector<std::string> find_by_index(const std::string& index_name, const std::string& key) const;
    std::vector<std::string> find_by_index(const std::vector<std::string>& field_names, const std::vector<std::string>& values) const;
    std::vector<std::string> find_by_index(const std::vector<std::string>& field_names) const;

    void save_indexes(const std::string& data_dir);
    void load_indexes(const std::string& data_dir);

private:
    std::string get_index_name(const std::vector<std::string>& field_names) const;
    std::string get_composite_key(const std::vector<std::string>& field_names, const Document& doc) const;

    // Maps an index name (e.g., "lastname_firstname") to a B+ tree instance.
    // The B+ tree maps a composite key (e.g., "Smith\0John") to a string
    // containing a JSON array of document IDs (e.g., "[\"doc1\", \"doc2\"]").
    std::map<std::string, std::unique_ptr<BTree<std::string, std::string>>> indexes_;
    // Maps an index name to the list of fields it covers.
    std::map<std::string, std::vector<std::string>> index_fields_;
};

} // namespace Storage
} // namespace TissDB
