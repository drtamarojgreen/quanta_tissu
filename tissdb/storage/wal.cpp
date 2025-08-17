#include "wal.h"
#include "../common/serialization.h"
#include "../common/binary_stream_buffer.h"
#include <iostream>
#include <vector>

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

    std::vector<uint8_t> buffer;
    BinaryStreamBuffer bsb(buffer);

    // Write entry data to an in-memory buffer first
    bsb.write(entry.type);
    bsb.write(entry.transaction_id);
    bsb.write_string(entry.document_id);

    if (entry.type == LogEntryType::PUT) {
        std::vector<uint8_t> doc_bytes = TissDB::serialize(entry.doc);
        bsb.write_bytes(doc_bytes);
    } else {
        size_t zero_len = 0;
        bsb.write(zero_len);
    }

    uint32_t checksum = Common::crc32(buffer.data(), buffer.size());
    uint32_t entry_size = buffer.size();

    // Write size, data, and checksum to the file
    BinaryStreamBuffer file_bsb(log_file);
    file_bsb.write(entry_size);
    log_file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
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
            // Read entry size
            bsb.read(entry_size);

            // Read entry data
            entry_data.resize(entry_size);
            input_log_file.read(reinterpret_cast<char*>(entry_data.data()), entry_size);
            if (static_cast<uint32_t>(input_log_file.gcount()) != entry_size) {
                std::cerr << "WAL data corruption: could not read full entry. Recovery stopped." << std::endl;
                break;
            }

            // Read checksum
            bsb.read(stored_checksum);
            if (input_log_file.fail()) {
                std::cerr << "WAL data corruption: could not read checksum. Recovery stopped." << std::endl;
                break;
            }

            // Verify checksum
            uint32_t calculated_checksum = Common::crc32(entry_data.data(), entry_data.size());
            if (stored_checksum != calculated_checksum) {
                std::cerr << "WAL checksum mismatch. Data corruption detected. Recovery stopped." << std::endl;
                break;
            }

            // Parse the entry from the verified data buffer
            LogEntry entry;
            BinaryStreamBuffer entry_bsb(entry_data);
            entry_bsb.read(entry.type);
            entry_bsb.read(entry.transaction_id);
            entry.document_id = entry_bsb.read_string();
            if (entry.type == LogEntryType::PUT) {
                entry.doc = TissDB::deserialize(entry_bsb.read_bytes());
            } else {
                entry_bsb.read_bytes(); // Consume the 0-length marker
                entry.doc = Document{};
            }
            recovered_entries.push_back(entry);

        } catch (const std::exception& e) {
            std::cerr << "Error during WAL recovery: " << e.what() << ". Recovery stopped." << std::endl;
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
    // Truncate the file by opening it in truncate mode
    log_file.open(log_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to clear WAL file: " + log_path);
    }
}

} // namespace Storage
} // namespace TissDB
