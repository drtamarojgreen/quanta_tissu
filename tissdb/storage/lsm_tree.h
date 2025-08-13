#pragma once

#include "memtable.h"
#include "wal.h"
#include "../common/document.h"
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include "sstable.h" // Include the new SSTable header

namespace TissDB {
namespace Storage {

// The LSMTree class is the main entry point for the storage engine.
// It orchestrates the Memtable, Write-Ahead Log (WAL), and the flushing
// of data to sorted-string-table (SSTable) files on disk.
class LSMTree {
public:
    // Initializes the storage engine in the given data directory.
    explicit LSMTree(const std::string& data_dir);

    // Inserts or updates a document. The write is first recorded in the WAL,
    // then inserted into the memtable.
    void put(const std::string& key, const Document& doc);

    // Retrieves a document by its key.
    // It searches in this order: memtable, then disk-based SSTables.
    std::optional<Document> get(const std::string& key);

    // Deletes a document by writing a tombstone.
    void del(const std::string& key);

private:
    // Flushes the current memtable to a new SSTable on disk when it's full.
    void flush_memtable();

    std::string data_directory_;
    std::unique_ptr<Memtable> memtable_;
    std::unique_ptr<WriteAheadLog> wal_;

    // A list of all SSTables currently managed by the LSM Tree, ordered from newest to oldest.
    std::vector<std::unique_ptr<SSTable>> sstables_;
};

} // namespace Storage
} // namespace TissDB
