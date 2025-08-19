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
#include "memtable.h"

namespace TissDB {
namespace Storage {

class LSMTree; // Forward declaration

// A Collection is an in-memory sorted data structure that holds all documents for a single collection.
// It is managed by the Database class.
class Collection : public Memtable {
public:
    Collection(const std::string& name, LSMTree* parent_db);
    Collection(const std::string& name, const std::string& path, LSMTree* parent_db);

    void set_schema(const TissDB::Schema& schema);
    void create_index(const std::vector<std::string>& field_names);
    void shutdown();

    // Indexing-related methods
    std::vector<std::string> find_by_index(const std::string& field_name, const std::string& value) const;
    std::vector<std::string> find_by_index(const std::vector<std::string>& field_names, const std::vector<std::string>& values) const;
    bool has_index(const std::vector<std::string>& field_names) const;
    std::vector<std::vector<std::string>> get_available_indexes() const;

private:
    std::string name_;
    TissDB::Schema schema_;
    LSMTree* parent_db_; // Pointer to the parent database
    std::unique_ptr<Indexer> indexer_;
};

} // namespace Storage
} // namespace TissDB