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
        LOG_ERROR("Failed to open WAL file: " + log_path);
        throw std::runtime_error("Failed to open WAL file: " + log_path);
    }
    LOG_INFO("Write-Ahead Log opened at: " + log_path);
}

WriteAheadLog::~WriteAheadLog() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void WriteAheadLog::append(const LogEntry& entry) {
    if (!log_file.is_open()) {
        LOG_ERROR("Attempted to append to closed WAL file.");
        throw std::runtime_error("WAL file is not open.");
    }

    std::string entry_type_str = (entry.type == LogEntryType::PUT) ? "PUT" : "DELETE";
    LOG_DEBUG("Appending to WAL: " + entry_type_str + " for doc ID: " + entry.document_id);

    std::stringstream buffer_stream;
    BinaryStreamBuffer bsb(static_cast<std::ostream&>(buffer_stream));

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

    std::string buffer_str = buffer_stream.str();
    uint32_t checksum = Common::crc32(buffer_str.data(), buffer_str.size());
    uint32_t entry_size = buffer_str.size();

    // Write size, data, and checksum to the file
    BinaryStreamBuffer file_bsb(log_file);
    file_bsb.write(entry_size);
    log_file.write(buffer_str.data(), entry_size);
    file_bsb.write(checksum);

    log_file.flush();
}

std::vector<LogEntry> WriteAheadLog::recover() {
    LOG_INFO("Starting WAL recovery from: " + log_path);
    std::vector<LogEntry> recovered_entries;
    std::ifstream input_log_file(log_path, std::ios::binary);
    if (!input_log_file.is_open()) {
        LOG_WARNING("WAL file not found for recovery. Assuming clean start.");
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
                LOG_ERROR("WAL data corruption: could not read full entry. Recovery stopped.");
                break;
            }

            // Read checksum
            bsb.read(stored_checksum);
            if (input_log_file.fail()) {
                LOG_ERROR("WAL data corruption: could not read checksum. Recovery stopped.");
                break;
            }

            // Verify checksum
            uint32_t calculated_checksum = Common::crc32(entry_data.data(), entry_data.size());
            if (stored_checksum != calculated_checksum) {
                LOG_ERROR("WAL checksum mismatch. Data corruption detected. Recovery stopped.");
                break;
            }

            // Parse the entry from the verified data buffer
            LogEntry entry;
            std::string entry_data_str(entry_data.begin(), entry_data.end());
            std::istringstream entry_stream(entry_data_str);
            BinaryStreamBuffer entry_bsb(entry_stream);
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
            LOG_ERROR("Error during WAL recovery: " + std::string(e.what()) + ". Recovery stopped.");
            break;
        }
    }
    input_log_file.close();
    LOG_INFO("WAL recovery finished. Recovered " + std::to_string(recovered_entries.size()) + " entries.");
    return recovered_entries;
}

void WriteAheadLog::clear() {
    if (log_file.is_open()) {
        log_file.close();
    }
    LOG_INFO("Clearing Write-Ahead Log: " + log_path);
    // Truncate the file by opening it in truncate mode
    log_file.open(log_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!log_file.is_open()) {
        LOG_ERROR("Failed to clear WAL file: " + log_path);
        throw std::runtime_error("Failed to clear WAL file: " + log_path);
    }
}

} // namespace Storage
} // namespace TissDB
