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

enum class IndexType {
    String,
    Timestamp
};

// The Indexer class manages all B+ tree indexes for the database.
class Indexer {
public:
    Indexer() = default;

    void create_index(const std::vector<std::string>& field_names, bool is_unique = false, IndexType type = IndexType::String);
    void create_timestamp_index(const std::vector<std::string>& field_names, bool is_unique = false);
    bool has_index(const std::vector<std::string>& field_names) const;
    void update_indexes(const std::string& document_id, const Document& doc);
    void remove_from_indexes(const std::string& document_id, const Document& doc);
    std::vector<std::string> find_by_index(const std::string& index_name, const Value& key) const;
    std::vector<std::string> find_by_index(const std::string& index_name, const std::string& value) const;
    std::vector<std::string> find_by_index(const std::vector<std::string>& field_names, const std::vector<Value>& values) const;
    std::vector<std::string> find_by_index(const std::vector<std::string>& field_names) const;
    std::vector<std::string> find_by_timestamp_range(const std::string& index_name, int64_t start_key, int64_t end_key) const;

    void save_indexes(const std::string& data_dir);
    void load_indexes(const std::string& data_dir);
    std::vector<std::vector<std::string>> get_available_indexes() const;

private:
    using BTreeVariant = std::variant<
        std::shared_ptr<BTree<std::string, std::string>>,
        std::shared_ptr<BTree<int64_t, std::string>>
    >;

    std::string get_index_name(const std::vector<std::string>& field_names) const;
    std::string get_composite_key(const std::vector<std::string>& field_names, const Document& doc) const;

    // Maps an index name (e.g., "lastname_firstname") to a B+ tree instance.
    // The B+ tree maps a composite key (e.g., "Smith\0John") to a string
    // containing a JSON array of document IDs (e.g., "[\"doc1\", \"doc2\"]").
    std::map<std::string, std::shared_ptr<BTree<std::string, std::string>>> indexes_;

    // Specialized B-Tree for timestamp indexes.
    std::map<std::string, std::shared_ptr<BTree<int64_t, std::string>>> timestamp_indexes_;

    // Maps an index name to the list of fields it covers.
    std::map<std::string, std::vector<std::string>> index_fields_;
    // Maps an index name to its type.
    std::map<std::string, IndexType> index_types_;
    // Maps an index name to whether it's a unique index.
    std::map<std::string, bool> index_uniqueness_;
};

} // namespace Storage
} // namespace TissDB
