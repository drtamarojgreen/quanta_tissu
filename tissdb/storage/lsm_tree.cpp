#include "lsm_tree.h"
#include "../common/serialization.h" // For TissDB::deserialize
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <vector>

namespace TissDB {
namespace Storage {

// A reasonable threshold for flushing the memtable to disk.
const size_t MEMTABLE_FLUSH_THRESHOLD = 4 * 1024 * 1024; // 4MB

LSMTree::LSMTree(const std::string& data_dir) : data_directory_(data_dir) {
    std::filesystem::create_directories(data_directory_);

    wal_ = std::make_unique<WriteAheadLog>(data_directory_ + "/wal.log");
    memtable_ = std::make_unique<Memtable>();

    // Load existing SSTables from the data directory on startup.
    std::vector<std::string> sstable_paths;
    for (const auto& entry : std::filesystem::directory_iterator(data_directory_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".db" && entry.path().filename().string().rfind("sstable_", 0) == 0) {
            sstable_paths.push_back(entry.path().string());
        }
    }
    // Sort paths descending based on the timestamp in the filename to have newest files first.
    std::sort(sstable_paths.rbegin(), sstable_paths.rend());

    for(const auto& path : sstable_paths) {
        sstables_.push_back(std::make_unique<SSTable>(path));
    }

    // A full implementation would also recover from the WAL here to rebuild the memtable
    // in case of a crash before a flush.
}

void LSMTree::put(const std::string& key, const Document& doc) {
    LogEntry entry;
    entry.type = LogEntryType::PUT;
    entry.document_id = key;
    entry.doc = doc;
    wal_->append(entry);

    memtable_->put(key, doc);

    if (memtable_->approximate_size() > MEMTABLE_FLUSH_THRESHOLD) {
        flush_memtable();
    }
}

std::optional<Document> LSMTree::get(const std::string& key) {
    // The read-path for an LSM-Tree:
    // 1. Check the Memtable first, as it contains the most recent data.
    auto mem_result = memtable_->get(key);
    if (mem_result.has_value()) {
        if (mem_result.value() == nullptr) { // Tombstone found
            return std::nullopt;
        }
        return *(mem_result.value()); // Document found in memtable
    }

    // 2. If not in memtable, search the on-disk SSTables, from newest to oldest.
    for (const auto& sstable : sstables_) {
        auto sst_result = sstable->find(key);
        if (sst_result.has_value()) {
            // The key was found in this SSTable.
            if (sst_result->empty()) {
                // An empty vector signifies a tombstone. The document is deleted.
                return std::nullopt;
            }
            // Found the serialized document data. Deserialize and return it.
            return TissDB::deserialize(*sst_result);
        }
        // If sst_result is nullopt, the key is not in this SSTable, so we continue to the next.
    }

    // 3. If the key is not found in the memtable or any SSTable, it doesn't exist.
    return std::nullopt;
}

void LSMTree::del(const std::string& key) {
    LogEntry entry;
    entry.type = LogEntryType::DELETE;
    entry.document_id = key;
    wal_->append(entry);

    memtable_->del(key);

    if (memtable_->approximate_size() > MEMTABLE_FLUSH_THRESHOLD) {
        flush_memtable();
    }
}

void LSMTree::flush_memtable() {
    // 1. Write the contents of the current (full) memtable to a new SSTable file.
    //    The static method handles file creation and returns the new path.
    std::string new_sstable_path = SSTable::write_from_memtable(data_directory_, *memtable_);

    // 2. Create a new SSTable object representing the new file and add it to the front
    //    of our list to maintain the newest-to-oldest order.
    sstables_.insert(sstables_.begin(), std::make_unique<SSTable>(new_sstable_path));

    // 3. Atomically swap the old memtable with a new empty one for incoming writes.
    memtable_ = std::make_unique<Memtable>();

    // 4. Since the data is now safely persisted in an SSTable, we can clear the
    //    Write-Ahead Log.
    wal_->clear();

    std::cout << "Memtable flushed to " << new_sstable_path << std::endl;
}

} // namespace Storage
} // namespace TissDB
