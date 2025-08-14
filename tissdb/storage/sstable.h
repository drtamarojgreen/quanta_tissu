#pragma once

#include "memtable.h"
#include "../common/document.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <optional>

namespace TissDB {
namespace Storage {

// Represents a single, immutable, sorted on-disk table.
class SSTable {
public:
    // Opens an existing SSTable file and loads its index into memory.
    SSTable(const std::string& path);

    // Searches for a key within this SSTable file.
    // Returns the serialized document data if found.
    // Returns a nullopt if the key is not found.
    // Returns an empty vector to represent a tombstone.
    std::optional<std::vector<uint8_t>> find(const std::string& key);

    // Scans all documents in the SSTable.
    std::vector<Document> scan();

    // Static method to create a new SSTable file from a Memtable.
    // Returns the path to the newly created SSTable file.
    static std::string write_from_memtable(const std::string& data_dir, const Memtable& memtable);

    // Static method to merge multiple SSTables into a new one.
    // Returns the path to the newly created SSTable file.
    static std::string merge(const std::string& data_dir, const std::vector<SSTable*>& sstables);

    const std::string& get_path() const { return file_path_; }

private:
    void load_index();

    std::string file_path_;
    std::ifstream file_stream_;
    // The sparse index maps a key to its offset in the file.
    // This allows for efficient lookups without reading the whole file.
    std::map<std::string, uint64_t> sparse_index_;
};

} // namespace Storage
} // namespace TissDB