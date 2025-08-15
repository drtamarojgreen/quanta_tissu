#pragma once

#include <cstddef>
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <filesystem>
#include <thread>
#include <atomic>

#include "memtable.h"
#include "wal.h"
#include "sstable.h"
#include "indexer.h"
#include "../common/document.h"
#include "../common/schema.h"
#include "../common/schema_validator.h"

namespace TissDB {
namespace Storage {

// A reasonable threshold for flushing the memtable to disk.
const size_t COLLECTION_MEMTABLE_FLUSH_THRESHOLD = 4 * 1024 * 1024; // 4MB

class Collection {
public:
    explicit Collection(const std::string& collection_path);
    ~Collection();

    void set_schema(const Schema& schema);
    void put(const std::string& key, const Document& doc);
    std::optional<Document> get(const std::string& key);
    void del(const std::string& key);
    std::vector<Document> scan();
    void shutdown();
    void create_index(const std::vector<std::string>& field_names);
    bool has_index(const std::vector<std::string>& field_names) const;
    std::vector<std::string> find_by_index(const std::string& field_name, const std::string& value);

private:
    void flush_memtable();
    void start_compaction_thread();
    void stop_compaction_thread();
    void compact();

    std::string collection_path_;
    std::unique_ptr<Memtable> memtable_;
    std::unique_ptr<WriteAheadLog> wal_;
    Indexer indexer_;
    std::vector<std::unique_ptr<SSTable>> sstables_;
    std::thread compaction_thread_;
    std::atomic<bool> stop_compaction_;
    std::optional<Schema> schema_;
};

} // namespace Storage
} // namespace TissDB