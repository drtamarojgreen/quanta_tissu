#include "collection.h"
#include "../common/serialization.h"
#include "../common/schema_validator.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <filesystem> // Added for std::filesystem

namespace TissDB {
namespace Storage {



Collection::Collection(const std::string& collection_path) : collection_path_(collection_path), stop_compaction_(false) {
    std::filesystem::create_directories(collection_path_);

    wal_ = std::make_unique<WriteAheadLog>(collection_path_ + "/wal.log");
    memtable_ = std::make_unique<Memtable>();

    // Load existing SSTables from the data directory on startup.
    std::vector<std::string> sstable_paths;
    for (const auto& entry : std::filesystem::directory_iterator(collection_path_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".db" && entry.path().filename().string().rfind("sstable_", 0) == 0) {
            sstable_paths.push_back(entry.path().string());
        }
    }
    // Sort paths descending based on the timestamp in the filename to have newest files first.
    std::sort(sstable_paths.rbegin(), sstable_paths.rend());

    for(const auto& path : sstable_paths) {
        sstables_.push_back(std::make_unique<SSTable>(path));
    }

    indexer_.load_indexes(collection_path_);

    // Recover from WAL
    auto recovered_entries = wal_->recover();
    for (const auto& entry : recovered_entries) {
        if (entry.type == LogEntryType::PUT) {
            memtable_->put(entry.document_id, entry.doc);
        } else if (entry.type == LogEntryType::DELETE) {
            memtable_->del(entry.document_id);
        }
    }
    wal_->clear();

    start_compaction_thread();
}

Collection::~Collection() {
    stop_compaction_thread();
}

void Collection::set_schema(const Schema& schema) {
    schema_ = std::make_unique<Schema>(schema);
    schema_validator_ = std::make_unique<SchemaValidator>(indexer_);
}

void Collection::put(const std::string& key, const Document& doc) {
    if (schema_validator_) {
        schema_validator_->validate(doc, *schema_);
    }

    LogEntry entry;
    entry.type = LogEntryType::PUT;
    entry.document_id = key;
    entry.doc = doc;
    wal_->append(entry);

    memtable_->put(key, doc);
    indexer_.update_indexes(key, doc);

    if (memtable_->approximate_size() > COLLECTION_MEMTABLE_FLUSH_THRESHOLD) {
        flush_memtable();
    }
}

std::optional<Document> Collection::get(const std::string& key) {
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
            return ::TissDB::deserialize(*sst_result);
        }
        // If sst_result is nullopt, the key is not in this SSTable, so we continue to the next.
    }

    // 3. If the key is not found in the memtable or any SSTable, it doesn't exist.
    return std::nullopt;
}

void Collection::del(const std::string& key) {
    LogEntry entry;
    entry.type = LogEntryType::DELETE;
    entry.document_id = key;
    wal_->append(entry);

    auto doc_to_delete = get(key);
    if (doc_to_delete) {
        indexer_.remove_from_indexes(key, *doc_to_delete);
    }

    memtable_->del(key);

    if (memtable_->approximate_size() > COLLECTION_MEMTABLE_FLUSH_THRESHOLD) {
        flush_memtable();
    }
}

std::vector<Document> Collection::scan() {
    std::map<std::string, Document> all_docs;

    // 1. Scan all SSTables from oldest to newest
    for (auto it = sstables_.rbegin(); it != sstables_.rend(); ++it) {
        auto sstable_docs = (*it)->scan();
        for (const auto& doc : sstable_docs) {
            all_docs[doc.id] = doc;
        }
    }

    // 2. Scan the memtable
    auto memtable_docs = memtable_->scan();
    for (const auto& doc : memtable_docs) {
        all_docs[doc.id] = doc;
    }

    // 3. Collect all non-tombstone documents
    std::vector<Document> result;
    for (const auto& pair : all_docs) {
        if (!pair.second.elements.empty()) {
            result.push_back(pair.second);
        }
    }

    return result;
}

void Collection::shutdown() {
    wal_->clear();
    indexer_.save_indexes(collection_path_);
}

void Collection::create_index(const std::vector<std::string>& field_names) {
    indexer_.create_index(field_names);
}

std::vector<std::string> Collection::find_by_index(const std::string& field_name, const std::string& value) {
    return indexer_.find_by_index(field_name, value);
}

std::vector<std::string> Collection::find_by_index(const std::vector<std::string>& field_names, const std::vector<std::string>& values) const {
    return indexer_.find_by_index(field_names, values);
}

bool Collection::has_index(const std::vector<std::string>& field_names) const {
    return indexer_.has_index(field_names);
}

const Indexer& Collection::get_indexer() const {
    return indexer_;
}

void Collection::flush_memtable() {
    std::string new_sstable_path = SSTable::write_from_memtable(collection_path_, *memtable_);
    sstables_.insert(sstables_.begin(), std::make_unique<SSTable>(new_sstable_path));
    memtable_ = std::make_unique<Memtable>();
    wal_->clear();
    std::cout << "Memtable flushed to " << new_sstable_path << std::endl;
}

void Collection::start_compaction_thread() {
    compaction_thread_ = std::thread([this]() {
        while (!stop_compaction_) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            compact();
        }
    });
}

void Collection::stop_compaction_thread() {
    stop_compaction_ = true;
    if (compaction_thread_.joinable()) {
        compaction_thread_.join();
    }
}

void Collection::compact() {
    // Size-tiered compaction strategy:
    // 1. Find SSTables of similar size.
    // 2. Merge them into a new SSTable.
    // 3. Remove the old SSTables.

    if (sstables_.size() < 2) {
        return;
    }

    // For simplicity, we'll just merge the two oldest SSTables.
    // A more sophisticated strategy would consider table sizes and levels.
    auto& sstable1 = sstables_[sstables_.size() - 1];
    auto& sstable2 = sstables_[sstables_.size() - 2];

    std::cout << "Compacting " << sstable1->get_path() << " and " << sstable2->get_path() << std::endl;

    // Merge the two SSTables
    std::string new_sstable_path = SSTable::merge(collection_path_, {sstable1.get(), sstable2.get()});

    // Create a new vector of SSTables, excluding the compacted ones and adding the new one.
    std::vector<std::unique_ptr<SSTable>> new_sstables;
    new_sstables.push_back(std::make_unique<SSTable>(new_sstable_path));
    for (size_t i = 0; i < sstables_.size() - 2; ++i) {
        new_sstables.push_back(std::move(sstables_[i]));
    }

    // Delete the old SSTable files
    std::filesystem::remove(sstable1->get_path());
    std::filesystem::remove(sstable2->get_path());

    sstables_ = std::move(new_sstables);

    std::cout << "Compaction complete. New SSTable: " << new_sstable_path << std::endl;
}

} // namespace Storage
} // namespace TissDB
