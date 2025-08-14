#include "wal.h"
#include "../common/serialization.h"
#include <iostream>

namespace TissDB {
namespace Storage {

WriteAheadLog::WriteAheadLog(const std::string& path) : log_path(path) {
    log_file.open(log_path, std::ios::app | std::ios::binary);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open WAL file: " + log_path);
    }
}

WriteAheadLog::~WriteAheadLog() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void WriteAheadLog::append(const LogEntry& entry) {
    if (!log_file.is_open()) {
        throw std::runtime_error("WAL file is not open.");
    }

    // Write entry type
    log_file.write(reinterpret_cast<const char*>(&entry.type), sizeof(entry.type));

    // Write transaction ID
    log_file.write(reinterpret_cast<const char*>(&entry.transaction_id), sizeof(entry.transaction_id));

    // Write document ID
    size_t id_len = entry.document_id.length();
    log_file.write(reinterpret_cast<const char*>(&id_len), sizeof(id_len));
    log_file.write(entry.document_id.data(), id_len);

    // Write document data (if PUT operation)
    if (entry.type == LogEntryType::PUT) {
        std::vector<uint8_t> doc_bytes = TissDB::serialize(entry.doc);
        size_t doc_len = doc_bytes.size();
        log_file.write(reinterpret_cast<const char*>(&doc_len), sizeof(doc_len));
        log_file.write(reinterpret_cast<const char*>(doc_bytes.data()), doc_len);
    } else { // For DELETE operations, write a marker for no document data
        size_t zero_len = 0;
        log_file.write(reinterpret_cast<const char*>(&zero_len), sizeof(zero_len));
    }

    log_file.flush(); // Ensure data is written to disk immediately
}

std::vector<LogEntry> WriteAheadLog::recover() {
    std::vector<LogEntry> recovered_entries;
    std::ifstream input_log_file(log_path, std::ios::binary);
    if (!input_log_file.is_open()) {
        return recovered_entries; // No log file, nothing to recover
    }

    while (input_log_file.peek() != EOF) {
        LogEntry entry;
        // Read entry type
        input_log_file.read(reinterpret_cast<char*>(&entry.type), sizeof(entry.type));

        // Read transaction ID
        input_log_file.read(reinterpret_cast<char*>(&entry.transaction_id), sizeof(entry.transaction_id));

        // Read document ID
        size_t id_len;
        input_log_file.read(reinterpret_cast<char*>(&id_len), sizeof(id_len));
        entry.document_id.resize(id_len);
        input_log_file.read(&entry.document_id[0], id_len);

        // Read document data (if PUT operation)
        size_t doc_len;
        input_log_file.read(reinterpret_cast<char*>(&doc_len), sizeof(doc_len));
        if (entry.type == LogEntryType::PUT) {
            std::vector<uint8_t> doc_bytes(doc_len);
            input_log_file.read(reinterpret_cast<char*>(doc_bytes.data()), doc_len);
            entry.doc = TissDB::deserialize(doc_bytes);
        }
        recovered_entries.push_back(entry);
    }
    input_log_file.close();
    return recovered_entries;
}

void WriteAheadLog::clear() {
    if (log_file.is_open()) {
        log_file.close();
    }
    // Truncate the file by opening it in truncate mode
    log_file.open(log_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to clear WAL file: " + log_path);
    }
}

} // namespace Storage
} // namespace TissDB