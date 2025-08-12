#include "lsm_tree.h"
#include <filesystem>

namespace TissDB {
namespace Storage {

// A reasonable threshold for flushing the memtable to disk.
const size_t MEMTABLE_FLUSH_THRESHOLD = 4 * 1024 * 1024; // 4MB

LSMTree::LSMTree(const std::string& data_dir) : data_directory(data_dir) {
    std::filesystem::create_directories(data_directory);

    // Initialize the components.
    wal = std::make_unique<WriteAheadLog>(data_directory + "/wal.log");
    memtable = std::make_unique<Memtable>();

    // In a real database, we would recover state from the WAL here.
    // auto log_entries = wal->recover();
    // for (const auto& entry : log_entries) { ... }
}

void LSMTree::put(const std::string& key, const Document& doc) {
    // The write-path for an LSM-Tree:
    // 1. First, write the operation to the Write-Ahead Log for durability.
    //    If the server crashes, we can recover from this log.
    LogEntry entry;
    entry.type = LogEntryType::PUT;
    entry.document_id = key; // The key is needed for recovery
    entry.doc = doc;
    wal->append(entry);

    // 2. Second, write the data to the in-memory Memtable.
    memtable->put(key, doc);

    // 3. Finally, check if the memtable has grown too large.
    //    If so, it should be scheduled for flushing to disk.
    if (memtable->approximate_size() > MEMTABLE_FLUSH_THRESHOLD) {
        // flush_memtable(); // This complex operation will be implemented later.
    }
}

std::optional<Document> LSMTree::get(const std::string& key) {
    // The read-path for an LSM-Tree:
    // 1. Check the Memtable first, as it contains the most recent data.
    auto mem_result = memtable->get(key);
    if (mem_result.has_value()) {
        if (mem_result.value() == nullptr) {
            // A tombstone was found, which means the document is deleted.
            return std::nullopt;
        }
        // Document was found in the memtable.
        return *(mem_result.value());
    }

    // 2. If not in the memtable, search the on-disk SSTables.
    //    This search must go from the newest SSTable to the oldest.
    //    This functionality will be implemented in a future task.
    //    For now, if it's not in the memtable, it's not found.
    return std::nullopt;
}

void LSMTree::del(const std::string& key) {
    // Deletion follows the same write-path as a put.
    // 1. Write a "tombstone" record to the Write-Ahead Log.
    LogEntry entry;
    entry.type = LogEntryType::DELETE;
    entry.document_id = key;
    wal->append(entry);

    // 2. Write the tombstone to the Memtable.
    memtable->del(key);

    // 3. Check if the memtable needs flushing.
    if (memtable->approximate_size() > MEMTABLE_FLUSH_THRESHOLD) {
        // flush_memtable();
    }
}

void LSMTree::flush_memtable() {
    // This is a placeholder for the memtable flush operation.
    // In a real implementation, this would be a complex process:
    // 1. Swap the active memtable with a new empty one.
    // 2. Make the old memtable immutable.
    // 3. In a background thread:
    //    a. Write the sorted contents of the immutable memtable to a new SSTable file.
    //    b. When the write is complete, update a manifest file with the new SSTable.
    //    c. The corresponding WAL entries can now be safely discarded.
}

} // namespace Storage
} // namespace TissDB
