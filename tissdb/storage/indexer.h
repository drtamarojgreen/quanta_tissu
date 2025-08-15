#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "bpp_tree.h"

// Forward declaration of the btree class from the B++ tree library
namespace bpp {
    template<typename Key, typename Value>
    class btree;
}

#include "../common/document.h"

namespace TissDB {
namespace Storage {

// The Indexer class manages all indexes for the database.
// STUBBED: B+ Tree implementation is missing, using std::map instead.
class Indexer {
public:
    Indexer() = default;

    void create_index(const std::vector<std::string>& field_names);
    bool has_index(const std::vector<std::string>& field_names) const;
    void update_indexes(const std::string& document_id, const Document& doc);
    void remove_from_indexes(const std::string& document_id, const Document& doc);
    std::vector<std::string> find_by_index(const std::string& field_name, const std::string& value) const;

    void save_indexes(const std::string& data_dir) { /* no-op */ }
    void load_indexes(const std::string& data_dir) { /* no-op */ }

private:
    std::string get_index_name(const std::vector<std::string>& field_names) const;

    // STUB: Using a map of maps as a fake index since B+ tree is not implemented.
    // Outer map: index_name -> Inner map: indexed_value -> document_id
    std::map<std::string, std::map<std::string, std::string>> indexes_;
    std::map<std::string, std::vector<std::string>> index_fields_;
};

} // namespace Storage
} // namespace TissDB
