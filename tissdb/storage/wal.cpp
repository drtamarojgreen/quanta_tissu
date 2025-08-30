#include "wal.h"
#include "../common/log.h"
#include "../common/serialization.h"
#include "../common/binary_stream_buffer.h"
#include "../crypto/kms.h"
#include <iostream>
#include <vector>
#include <sstream>

namespace TissDB {
namespace Storage {

namespace {
// This is a placeholder for a proper dependency injection mechanism.
TissDB::Crypto::KeyManagementSystem& get_kms_instance() {
    // This master key should be loaded from a secure configuration or vault, not hardcoded.
    static TissDB::Crypto::Key master_key = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    };
    static TissDB::Crypto::KeyManagementSystem instance(master_key);
    return instance;
}
} // anonymous namespace

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

    switch (entry.type) {
        case LogEntryType::PUT:
            bsb.write_string(entry.collection_name);
            bsb.write_string(entry.document_id);
            bsb.write_bytes(TissDB::serialize(entry.doc));
            break;
        case LogEntryType::DELETE:
            bsb.write_string(entry.collection_name);
            bsb.write_string(entry.document_id);
            break;
        case LogEntryType::TXN_COMMIT: {
            bsb.write(static_cast<uint64_t>(entry.operations.size()));
            for (const auto& op : entry.operations) {
                bsb.write(op.type);
                bsb.write_string(op.collection_name);
                bsb.write_string(op.key);
                if (op.type == TissDB::Transactions::OperationType::PUT) {
                    bsb.write_bytes(TissDB::serialize(op.doc));
                }
            }
            break;
        }
        case LogEntryType::TXN_ABORT:
            // Only type and TID are needed.
            break;
        case LogEntryType::CREATE_COLLECTION:
        case LogEntryType::DELETE_COLLECTION:
            bsb.write_string(entry.collection_name);
            break;
    }

    std::string buffer_str = buffer_stream.str();

    // Encrypt the entire log entry
    auto dek = get_kms_instance().get_dek("wal_key"); // Use a dedicated key for the WAL
    Crypto::Buffer plaintext_buffer(buffer_str.begin(), buffer_str.end());
    Crypto::Buffer encrypted_buffer = get_kms_instance().encrypt(plaintext_buffer, dek);

    uint32_t checksum = Common::crc32(encrypted_buffer.data(), encrypted_buffer.size());
    uint32_t entry_size = encrypted_buffer.size();

    BinaryStreamBuffer file_bsb(log_file);
    file_bsb.write(entry_size);
    log_file.write(reinterpret_cast<const char*>(encrypted_buffer.data()), entry_size);
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

            // Decrypt the entry data
            auto dek = get_kms_instance().get_dek("wal_key");
            auto decrypted_buffer = get_kms_instance().decrypt(entry_data, dek);
            if (decrypted_buffer.empty() && !entry_data.empty()) {
                std::cerr << "WAL entry decryption failed. Recovery may be incomplete." << std::endl;
                break;
            }

            LogEntry entry;
            std::string entry_data_str(decrypted_buffer.begin(), decrypted_buffer.end());
            std::istringstream entry_stream(entry_data_str);
            BinaryStreamBuffer entry_bsb(entry_stream);
            entry_bsb.read(entry.type);
            entry_bsb.read(entry.transaction_id);

            switch (entry.type) {
                case LogEntryType::PUT:
                    entry.collection_name = entry_bsb.read_string();
                    entry.document_id = entry_bsb.read_string();
                    entry.doc = TissDB::deserialize(entry_bsb.read_bytes());
                    break;
                case LogEntryType::DELETE:
                    entry.collection_name = entry_bsb.read_string();
                    entry.document_id = entry_bsb.read_string();
                    break;
                case LogEntryType::TXN_COMMIT: {
                    uint64_t op_count;
                    entry_bsb.read(op_count);
                    for (uint64_t i = 0; i < op_count; ++i) {
                        TissDB::Transactions::Operation op;
                        entry_bsb.read(op.type);
                        op.collection_name = entry_bsb.read_string();
                        op.key = entry_bsb.read_string();
                        if (op.type == TissDB::Transactions::OperationType::PUT) {
                            op.doc = TissDB::deserialize(entry_bsb.read_bytes());
                        }
                        entry.operations.push_back(op);
                    }
                    break;
                }
                case LogEntryType::TXN_ABORT:
                    break; // No more data to read
                case LogEntryType::CREATE_COLLECTION:
                case LogEntryType::DELETE_COLLECTION:
                    entry.collection_name = entry_bsb.read_string();
                    break;
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

void WriteAheadLog::shutdown() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

} // namespace Storage
} // namespace TissDB
