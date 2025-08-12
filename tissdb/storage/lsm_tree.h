#pragma once

#include "memtable.h"
#include "wal.h"
#include "../common/document.h"
#include <string>
#include <memory>
#include <optional>
#include <vector>

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

    std::string data_directory;
    std::unique_ptr<Memtable> memtable;
    std::unique_ptr<WriteAheadLog> wal;

    // In a real implementation, this would hold metadata about the SSTable files on disk.
    // std::vector<SSTableMeta> sstables;
};

} // namespace Storage
} // namespace TissDB
