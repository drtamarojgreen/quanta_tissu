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
class Indexer {
public:
    Indexer();

    // Creates a new index on a specific field.
    void create_index(const std::vector<std::string>& field_names);

    // Checks if an index exists for a given field.
    bool has_index(const std::vector<std::string>& field_names) const;

    // Updates all relevant indexes with the data from a new document.
    void update_indexes(const std::string& document_id, const Document& doc);

    // Removes a document from all relevant indexes.
    void remove_from_indexes(const std::string& document_id, const Document& doc);

    // Finds a list of document IDs using an index.
    std::vector<std::string> find_by_index(const std::string& field_name, const std::string& value) const;

    void save_indexes(const std::string& data_dir);
    void load_indexes(const std::string& data_dir);

private:
    std::string get_index_name(const std::vector<std::string>& field_names) const;

    std::map<std::string, std::unique_ptr<bpp::btree<std::string, std::string>>> indexes_;
    std::map<std::string, std::vector<std::string>> index_fields_;
};

} // namespace Storage
} // namespace TissDB
