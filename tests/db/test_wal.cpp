
#include "test_framework.h"
#include "../../../tissdb/storage/wal.h"
#include "../../../tissdb/common/document.h"
#include <filesystem>

TEST_CASE(WALAppendAndRecover) {
    std::string wal_path = "test_wal.log";
    if (std::filesystem::exists(wal_path)) {
        std::filesystem::remove(wal_path);
    }

    // Append some entries
    {
        TissDB::Storage::WriteAheadLog wal(wal_path);

        TissDB::Document doc1;
        doc1.id = "doc1";
        TissDB::Element elem1_1; elem1_1.key = "field1"; elem1_1.value = std::string("value1");
        doc1.elements.push_back(elem1_1);

        TissDB::Document doc2;
        doc2.id = "doc2";
        TissDB::Element elem2_1; elem2_1.key = "field2"; elem2_1.value = 123.45;
        doc2.elements.push_back(elem2_1);

        TissDB::Storage::LogEntry entry1 = {TissDB::Storage::LogEntryType::PUT, doc1, doc1.id};
        TissDB::Storage::LogEntry entry2 = {TissDB::Storage::LogEntryType::PUT, doc2, doc2.id};
        TissDB::Storage::LogEntry entry3 = {TissDB::Storage::LogEntryType::DELETE, TissDB::Document{}, "doc1"};

        wal.append(entry1);
        wal.append(entry2);
        wal.append(entry3);
    }

    // Recover entries
    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        std::vector<TissDB::Storage::LogEntry> recovered_entries = wal.recover();

        ASSERT_EQ(3, recovered_entries.size());

        ASSERT_EQ(TissDB::Storage::LogEntryType::PUT, recovered_entries[0].type);
        ASSERT_EQ("doc1", recovered_entries[0].document_id);
        ASSERT_EQ("value1", std::get<std::string>(recovered_entries[0].doc.elements[0].value));

        ASSERT_EQ(TissDB::Storage::LogEntryType::PUT, recovered_entries[1].type);
        ASSERT_EQ("doc2", recovered_entries[1].document_id);
        ASSERT_EQ(123.45, std::get<double>(recovered_entries[1].doc.elements[0].value));

        ASSERT_EQ(TissDB::Storage::LogEntryType::DELETE, recovered_entries[2].type);
        ASSERT_EQ("doc1", recovered_entries[2].document_id);
    }

    std::filesystem::remove(wal_path);
}

TEST_CASE(WALClear) {
    std::string wal_path = "test_wal_clear.log";
    if (std::filesystem::exists(wal_path)) {
        std::filesystem::remove(wal_path);
    }

    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        TissDB::Document doc;
        doc.id = "temp_doc";
        TissDB::Storage::LogEntry entry = {TissDB::Storage::LogEntryType::PUT, doc, doc.id};
        wal.append(entry);
    }

    // Check if content exists
    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        ASSERT_EQ(1, wal.recover().size());
    }

    // Clear and check again
    {
        TissDB::Storage::WriteAheadLog wal(wal_path);
        wal.clear();
        ASSERT_EQ(0, wal.recover().size());
    }

    std::filesystem::remove(wal_path);
}
