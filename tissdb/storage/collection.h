
#pragma once

#include "memtable.h"
#include "wal.h"
#include "sstable.h"
#include "indexer.h"
#include "../common/document.h"
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <filesystem>

namespace TissDB {
namespace Storage {

// A reasonable threshold for flushing the memtable to disk.
const size_t COLLECTION_MEMTABLE_FLUSH_THRESHOLD = 4 * 1024 * 1024; // 4MB

class Collection {
public:
    explicit Collection(const std::string& collection_path);

    void put(const std::string& key, const Document& doc);
    std::optional<Document> get(const std::string& key);
    void del(const std::string& key);
    std::vector<Document> scan();
    void shutdown();
    void create_index(const std::string& field_name);
    std::vector<std::string> find_by_index(const std::string& field_name, const std::string& value);

private:
    void flush_memtable();

    std::string collection_path_;
    std::unique_ptr<Memtable> memtable_;
    std::unique_ptr<WriteAheadLog> wal_;
    Indexer indexer_;
    std::vector<std::unique_ptr<SSTable>> sstables_;
};

} // namespace Storage
} // namespace TissDB
