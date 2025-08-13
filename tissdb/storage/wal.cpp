#include "wal.h"
#include "../common/serialization.h" // For TissDB::serialize
#include <stdexcept>
#include <vector>
#include <cstring> // For memcpy

namespace TissDB {
namespace Storage {

// Anonymous namespace for helpers private to this file.
namespace {

// Helper to write a length-prefixed record to the log file.
void write_record(std::ofstream& os, const std::vector<uint8_t>& data) {
    size_t data_size = data.size();
    os.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
    os.write(reinterpret_cast<const char*>(data.data()), data_size);
}

} // anonymous namespace


WriteAheadLog::WriteAheadLog(const std::string& path) : log_path(path) {
    // Open the file in binary append mode. This allows us to add to the end
    // of the log without overwriting existing content.
    log_file.open(log_path, std::ios::out | std::ios::binary | std::ios::app);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open WAL file for appending: " + path);
    }
}

WriteAheadLog::~WriteAheadLog() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void WriteAheadLog::append(const LogEntry& entry) {
    if (!log_file.is_open()) {
        throw std::runtime_error("WAL file is not open for writing.");
    }

    // Serialize the log entry into a byte vector.
    // For a PUT operation, this involves serializing the entire document.
    // For a DELETE, we just need the document ID.
    std::vector<uint8_t> entry_payload;
    entry_payload.push_back(static_cast<uint8_t>(entry.type));

    if (entry.type == LogEntryType::PUT) {
        std::vector<uint8_t> doc_bytes = TissDB::serialize(entry.doc);
        entry_payload.insert(entry_payload.end(), doc_bytes.begin(), doc_bytes.end());
    } else if (entry.type == LogEntryType::DELETE) {
        // For delete, the "document" is just its ID. We can use our string serialization.
        std::vector<uint8_t> id_bytes(entry.document_id.begin(), entry.document_id.end());
        size_t id_len = id_bytes.size();
        entry_payload.resize(1 + sizeof(id_len) + id_len);
        memcpy(entry_payload.data() + 1, &id_len, sizeof(id_len));
        memcpy(entry_payload.data() + 1 + sizeof(id_len), id_bytes.data(), id_len);
    }

    // Write the serialized entry as a single length-prefixed record to the log.
    write_record(log_file, entry_payload);

    // Flush the buffer to disk to ensure durability. This is critical for the WAL.
    log_file.flush();
}

std::vector<LogEntry> WriteAheadLog::recover() {
    // Placeholder: Recovery logic is complex. It involves reading the log file
    // from the beginning, parsing each length-prefixed record, deserializing
    // the payload back into a LogEntry, and adding it to a vector.
    // This will be implemented in a future task.
    return {};
}

void WriteAheadLog::clear() {
    // Close the current file handle and re-open the file in truncate mode
    // to effectively erase its contents.
    if (log_file.is_open()) {
        log_file.close();
    }
    // std::ios::trunc will delete the file content.
    log_file.open(log_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open WAL file for truncation: " + log_path);
    }
}

} // namespace Storage
} // namespace TissDB
