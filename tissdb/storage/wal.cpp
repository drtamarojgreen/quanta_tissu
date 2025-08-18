#include "wal.h"
#include "../common/log.h"
#include "../common/serialization.h"
#include "../common/binary_stream_buffer.h"
#include <iostream>
#include <vector>
#include <sstream>

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

    std::stringstream buffer_stream;
    BinaryStreamBuffer bsb(static_cast<std::ostream&>(buffer_stream));

    bsb.write(entry.type);
    bsb.write(entry.transaction_id);
    bsb.write_string(entry.collection_name);
    bsb.write_string(entry.document_id); // For collection ops, this can be empty

    if (entry.type == LogEntryType::PUT) {
        std::vector<uint8_t> doc_bytes = TissDB::serialize(entry.doc);
        bsb.write_bytes(doc_bytes);
    } else {
        // For DELETE, CREATE_COLLECTION, DELETE_COLLECTION, no doc is needed.
        size_t zero_len = 0;
        bsb.write(zero_len);
    }

    std::string buffer_str = buffer_stream.str();
    uint32_t checksum = Common::crc32(buffer_str.data(), buffer_str.size());
    uint32_t entry_size = buffer_str.size();

    BinaryStreamBuffer file_bsb(log_file);
    file_bsb.write(entry_size);
    log_file.write(buffer_str.data(), entry_size);
    file_bsb.write(checksum);

    log_file.flush();
}

std::vector<LogEntry> WriteAheadLog::recover() {
    std::vector<LogEntry> recovered_entries;
    std::ifstream input_log_file(log_path, std::ios::binary);
    if (!input_log_file.is_open()) {
        return recovered_entries;
    }

    BinaryStreamBuffer bsb(input_log_file);

    while (input_log_file.peek() != EOF) {
        uint32_t entry_size;
        uint32_t stored_checksum;
        std::vector<uint8_t> entry_data;

        try {
            bsb.read(entry_size);
            entry_data.resize(entry_size);
            input_log_file.read(reinterpret_cast<char*>(entry_data.data()), entry_size);
            if (static_cast<uint32_t>(input_log_file.gcount()) != entry_size) {
                break;
            }

            bsb.read(stored_checksum);
            if (input_log_file.fail()) {
                break;
            }

            if (stored_checksum != Common::crc32(entry_data.data(), entry_data.size())) {
                break;
            }

            LogEntry entry;
            std::string entry_data_str(entry_data.begin(), entry_data.end());
            std::istringstream entry_stream(entry_data_str);
            BinaryStreamBuffer entry_bsb(entry_stream);
            entry_bsb.read(entry.type);
            entry_bsb.read(entry.transaction_id);
            entry.collection_name = entry_bsb.read_string();
            entry.document_id = entry_bsb.read_string();

            if (entry.type == LogEntryType::PUT) {
                entry.doc = TissDB::deserialize(entry_bsb.read_bytes());
            } else {
                entry_bsb.read_bytes(); // Consume the empty bytes
                entry.doc = Document{};
            }
            recovered_entries.push_back(entry);

        } catch (const std::exception& e) {
            break;
        }
    }
    input_log_file.close();
    return recovered_entries;
}

void WriteAheadLog::clear() {
    if (log_file.is_open()) {
        log_file.close();
    }
    log_file.open(log_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to clear WAL file: " + log_path);
    }
}

} // namespace Storage
} // namespace TissDB
