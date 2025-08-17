#include "wal.h"
#include "../common/serialization.h"
#include "../common/binary_stream_buffer.h"
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

    BinaryStreamBuffer bsb(log_file);

    // Write entry type
    bsb.write(entry.type);

    // Write transaction ID
    bsb.write(entry.transaction_id);

    // Write document ID
    bsb.write_string(entry.document_id);

    // Write document data (if PUT operation)
    if (entry.type == LogEntryType::PUT) {
        std::vector<uint8_t> doc_bytes = TissDB::serialize(entry.doc);
        bsb.write_bytes(doc_bytes);
    } else { // For DELETE operations, write a marker for no document data
        size_t zero_len = 0;
        bsb.write(zero_len); // Write a zero length for tombstone value
    }

    log_file.flush(); // Ensure data is written to disk immediately
}

std::vector<LogEntry> WriteAheadLog::recover() {
    std::vector<LogEntry> recovered_entries;
    std::ifstream input_log_file(log_path, std::ios::binary);
    if (!input_log_file.is_open()) {
        return recovered_entries; // No log file, nothing to recover
    }

    BinaryStreamBuffer bsb(input_log_file);

    while (input_log_file.peek() != EOF) {
        LogEntry entry;
        try {
            // Read entry type
            bsb.read(entry.type);

            // Read transaction ID
            bsb.read(entry.transaction_id);

            // Read document ID
            entry.document_id = bsb.read_string();

            // Read document data
            if (entry.type == LogEntryType::PUT) {
                entry.doc = TissDB::deserialize(bsb.read_bytes());
            } else { // For DELETE operations
                // Consume the 0-length marker written for DELETE operations
                bsb.read_bytes();
                entry.doc = Document{};
            }
            recovered_entries.push_back(entry);
        } catch (const std::exception& e) {
            std::cerr << "Error during WAL recovery: " << e.what() << std::endl;
            break; // Stop recovery on error
        }
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
