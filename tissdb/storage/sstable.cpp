#include "sstable.h"
#include "../common/serialization.h"
#include "../common/binary_stream_buffer.h"
#include <iostream>
#include <chrono>
#include <map>

namespace TissDB {
namespace Storage {

// --- SSTable Public Methods ---

SSTable::SSTable(const std::string& path) : file_path_(path) {
    // In a real implementation, this would open the file and load the index.
    // For now, we'll open the file on-demand in find().
    // load_index();
}

std::optional<std::vector<uint8_t>> SSTable::find(const std::string& key) {
    std::ifstream sst_file(file_path_, std::ios::binary);
    if (!sst_file.is_open()) {
        return std::nullopt;
    }

    BinaryStreamBuffer bsb(sst_file);

    while (bsb.good() && !bsb.eof()) {
        try {
            std::string current_key = bsb.read_string();
            
            // Read value length marker
            size_t val_len_marker;
            bsb.read(val_len_marker);

            if (current_key == key) {
                if (val_len_marker == static_cast<size_t>(-1)) { // Tombstone marker
                    return std::vector<uint8_t>(); // Empty vector for tombstone
                }
                // Read the actual value bytes using the new safe method
                return bsb.read_bytes_with_length(val_len_marker);
            } else {
                // Key doesn't match, so skip over the value bytes to get to the next key.
                if (val_len_marker != static_cast<size_t>(-1)) {
                     // Use the new method to consume bytes, even if we don't need the return value
                     bsb.read_bytes_with_length(val_len_marker);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error during SSTable find: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    return std::nullopt; // Key not found
}

std::vector<Document> SSTable::scan() {
    std::vector<Document> documents;
    std::ifstream sst_file(file_path_, std::ios::binary);
    if (!sst_file.is_open()) {
        return documents;
    }

    BinaryStreamBuffer bsb(sst_file);

    while (bsb.good() && !bsb.eof()) {
        try {
            std::string current_key = bsb.read_string();
            
            size_t val_len_marker;
            bsb.read(val_len_marker);

            if (val_len_marker != static_cast<size_t>(-1)) { // Not a tombstone
                documents.push_back(deserialize(bsb.read_bytes_with_length(val_len_marker)));
            } else {
                // Tombstone
                Document tombstone;
                tombstone.id = current_key;
                documents.push_back(tombstone);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error during SSTable scan: " << e.what() << std::endl;
            break; // Stop scan on error
        }
    }
    return documents;
}

std::string SSTable::write_from_memtable(const std::string& data_dir, const Memtable& memtable) {
    // Generate a unique filename for the new SSTable using a timestamp.
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::string file_path = data_dir + "/sstable_" + std::to_string(timestamp) + ".db";

    std::ofstream sst_file(file_path, std::ios::binary | std::ios::trunc);
    if (!sst_file.is_open()) {
        throw std::runtime_error("Failed to create SSTable file: " + file_path);
    }

    BinaryStreamBuffer bsb(sst_file);

    // The memtable's map is already sorted by key, which is exactly what we need.
    const auto& data = memtable.get_all();
    for (const auto& pair : data) {
        // Write key (length-prefixed)
        bsb.write_string(pair.first);

        // Write value (length-prefixed)
        if (pair.second) { // If it's a document pointer
            std::vector<uint8_t> value_bytes = TissDB::serialize(*(pair.second));
            bsb.write_bytes(value_bytes);
        } else { // If it's a tombstone (nullptr)
            size_t tombstone_marker = static_cast<size_t>(-1);
            bsb.write(tombstone_marker);
        }
    }

    sst_file.close();
    return file_path;
}

std::string SSTable::merge(const std::string& data_dir, const std::vector<SSTable*>& sstables) {
    // Generate a unique filename for the new SSTable using a timestamp.
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::string file_path = data_dir + "/sstable_" + std::to_string(timestamp) + ".db";

    std::ofstream sst_file(file_path, std::ios::binary | std::ios::trunc);
    if (!sst_file.is_open()) {
        throw std::runtime_error("Failed to create SSTable file: " + file_path);
    }

    BinaryStreamBuffer bsb(sst_file);

    // Use a map to merge and sort all key-value pairs from the SSTables.
    // The map will automatically handle duplicates, keeping the value from the last SSTable (newest).
    std::map<std::string, std::optional<std::vector<uint8_t>>> merged_data;

    for (const auto& sstable : sstables) {
        std::ifstream current_sst_file(sstable->get_path(), std::ios::binary);
        if (!current_sst_file.is_open()) {
            continue;
        }

        BinaryStreamBuffer current_bsb(current_sst_file);

        while (current_bsb.good() && !current_bsb.eof()) {
            try {
                std::string current_key = current_bsb.read_string();
                
                size_t val_len_marker;
                current_bsb.read(val_len_marker);

                if (val_len_marker == static_cast<size_t>(-1)) { // Tombstone
                    merged_data[current_key] = std::nullopt;
                } else {
                    merged_data[current_key] = current_bsb.read_bytes_with_length(val_len_marker);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error during SSTable merge read: " << e.what() << std::endl;
                break; // Stop reading this SSTable on error
            }
        }
    }

    // Write the merged data to the new SSTable.
    for (const auto& pair : merged_data) {
        bsb.write_string(pair.first);

        if (pair.second.has_value()) {
            const auto& value_bytes = pair.second.value();
            bsb.write_bytes(value_bytes);
        } else {
            size_t tombstone_marker = static_cast<size_t>(-1);
            bsb.write(tombstone_marker);
        }
    }

    sst_file.close();
    return file_path;
}

void SSTable::load_index() {
    // Placeholder: In a real implementation, this would either read a separate
    // index file or build the sparse index by iterating through the keys in the
    // main sstable file, storing every Nth key and its offset.
}

} // namespace Storage
} // namespace TissDB
