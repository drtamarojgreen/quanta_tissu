#include "test_framework.h"
#include "../../tissdb/storage/wal.h"
#include "../../tissdb/storage/sstable.h"
#include "../../tissdb/storage/memtable.h"
#include "../../tissdb/common/document.h"
#include <filesystem>
#include <fstream>

// Helper function to corrupt a file by overwriting a byte at a specific offset
void corrupt_file(const std::string& path, long offset, char new_byte) {
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    if (file.is_open()) {
        file.seekp(offset);
        file.put(new_byte);
        file.close();
    }
}

TEST_CASE(WALChecksumValid) {
    std::string wal_path = "test_wal_checksum_valid.log";
    if (std::filesystem::exists(wal_path)) {
        std::filesystem::remove(wal_path);
    }

    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        TissDB::Document doc;
        doc.id = "doc1";
        TissDB::Storage::LogEntry entry = {TissDB::Storage::LogEntryType::PUT, -1, doc, doc.id};
        wal.append(entry);
    }

    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        std::vector<TissDB::Storage::LogEntry> recovered = wal.recover();
        ASSERT_EQ(1, recovered.size());
        ASSERT_EQ("doc1", recovered[0].document_id);
    }

    std::filesystem::remove(wal_path);
}

TEST_CASE(WALChecksumInvalid) {
    std::string wal_path = "test_wal_checksum_invalid.log";
    if (std::filesystem::exists(wal_path)) {
        std::filesystem::remove(wal_path);
    }

    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        TissDB::Document doc;
        doc.id = "doc1";
        TissDB::Storage::LogEntry entry = {TissDB::Storage::LogEntryType::PUT, -1, doc, doc.id};
        wal.append(entry);
    }

    // Corrupt the WAL file
    corrupt_file(wal_path, 5, 0xAB);

    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        std::vector<TissDB::Storage::LogEntry> recovered = wal.recover();
        // Recovery should stop at the corrupted entry
        ASSERT_EQ(0, recovered.size());
    }

    std::filesystem::remove(wal_path);
}

TEST_CASE(SSTableChecksumValid) {
    std::string data_dir = "test_sstable_checksum_valid";
    if (!std::filesystem::exists(data_dir)) {
        std::filesystem::create_directory(data_dir);
    }

    TissDB::Storage::Memtable memtable;
    TissDB::Document doc1;
    doc1.id = "doc1";
    memtable.put("doc1", std::make_shared<TissDB::Document>(doc1));

    std::string sstable_path = TissDB::Storage::SSTable::write_from_memtable(data_dir, memtable);

    // This should load successfully
    TissDB::Storage::SSTable sstable(sstable_path);
    auto result = sstable.find("doc1");
    ASSERT_TRUE(result.has_value());

    std::filesystem::remove_all(data_dir);
}

TEST_CASE(SSTableChecksumInvalid) {
    std::string data_dir = "test_sstable_checksum_invalid";
    if (!std::filesystem::exists(data_dir)) {
        std::filesystem::create_directory(data_dir);
    }

    TissDB::Storage::Memtable memtable;
    TissDB::Document doc1;
    doc1.id = "doc1";
    memtable.put("doc1", std::make_shared<TissDB::Document>(doc1));

    std::string sstable_path = TissDB::Storage::SSTable::write_from_memtable(data_dir, memtable);

    // Corrupt the SSTable file
    corrupt_file(sstable_path, 10, 0xAB);

    // This should fail to load
    TissDB::Storage::SSTable sstable(sstable_path);
    auto result = sstable.find("doc1");
    ASSERT_FALSE(result.has_value()); // The SSTable should be invalid and thus empty

    std::filesystem::remove_all(data_dir);
}
