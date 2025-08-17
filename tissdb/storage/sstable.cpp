#include "sstable.h"
#include "../common/serialization.h"
#include "../common/binary_stream_buffer.h"
#include <iostream>
#include <chrono>
#include <map>

namespace TissDB {
namespace Storage {

const int SSTABLE_INDEX_INTERVAL = 16; // Sample every 16th key for the sparse index

// --- SSTable Public Methods ---

SSTable::SSTable(const std::string& path) : file_path_(path) {
    file_stream_.open(file_path_, std::ios::binary);
    if (file_stream_.is_open()) {
        load_index();
    }
}

std::optional<std::vector<uint8_t>> SSTable::find(const std::string& key) {
    if (!file_stream_.is_open() || sparse_index_.empty()) {
        return std::nullopt;
    }

    // Find the last indexed key that is less than or equal to the target key.
    auto it = sparse_index_.upper_bound(key);
    uint64_t start_offset = 0;
    if (it != sparse_index_.begin()) {
        start_offset = std::prev(it)->second;
    }

    file_stream_.clear();
    file_stream_.seekg(start_offset);
    BinaryStreamBuffer bsb(file_stream_);

    while (file_stream_.peek() != EOF) {
        try {
            // Check if we've scanned past the next indexed key. If so, the key is not in this block.
            if (it != sparse_index_.end() && static_cast<uint64_t>(file_stream_.tellg()) >= it->second) {
                return std::nullopt;
            }

            std::string current_key = bsb.read_string();
            
            size_t val_len_marker;
            bsb.read(val_len_marker);

            if (current_key == key) {
                if (val_len_marker == static_cast<size_t>(-1)) { // Tombstone marker
                    return std::vector<uint8_t>();
                }
                return bsb.read_bytes_with_length(val_len_marker);
            } else if (current_key > key) {
                // We've scanned past where the key should be, so it doesn't exist.
                return std::nullopt;
            }

            // Key doesn't match, skip value bytes to get to the next entry.
            if (val_len_marker != static_cast<size_t>(-1)) {
                file_stream_.seekg(val_len_marker, std::ios_base::cur);
            }
        } catch (const std::ios_base::failure& e) {
            // This can happen if we read past the end of the file, which is a normal way to end the search.
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

    while (sst_file.peek() != EOF) {
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
    std::map<std::string, uint64_t> sparse_index;
    int key_count = 0;

    // The memtable's map is already sorted by key, which is exactly what we need.
    const auto& data = memtable.get_all();
    for (const auto& pair : data) {
        // Sample every Nth key for the sparse index
        if (key_count % SSTABLE_INDEX_INTERVAL == 0) {
            sparse_index[pair.first] = static_cast<uint64_t>(sst_file.tellp());
        }
        key_count++;

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

    // After writing all data, write the index block.
    uint64_t index_start_offset = static_cast<uint64_t>(sst_file.tellp());
    bsb.write(static_cast<size_t>(sparse_index.size()));
    for (const auto& pair : sparse_index) {
        bsb.write_string(pair.first);
        bsb.write(pair.second);
    }

    // Finally, write the footer containing the offset of the index block.
    bsb.write(index_start_offset);

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

        while (current_sst_file.peek() != EOF) {
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

    // Write the merged data to the new SSTable and create a sparse index.
    std::map<std::string, uint64_t> sparse_index;
    int key_count = 0;
    for (const auto& pair : merged_data) {
        if (key_count % SSTABLE_INDEX_INTERVAL == 0) {
            sparse_index[pair.first] = static_cast<uint64_t>(sst_file.tellp());
        }
        key_count++;

        bsb.write_string(pair.first);

        if (pair.second.has_value()) {
            const auto& value_bytes = pair.second.value();
            bsb.write_bytes(value_bytes);
        } else {
            size_t tombstone_marker = static_cast<size_t>(-1);
            bsb.write(tombstone_marker);
        }
    }

    // After writing all data, write the index block.
    uint64_t index_start_offset = static_cast<uint64_t>(sst_file.tellp());
    bsb.write(static_cast<size_t>(sparse_index.size()));
    for (const auto& pair : sparse_index) {
        bsb.write_string(pair.first);
        bsb.write(pair.second);
    }

    // Finally, write the footer containing the offset of the index block.
    bsb.write(index_start_offset);

    sst_file.close();
    return file_path;
}

void SSTable::load_index() {
    // The file_stream_ is already open from the constructor.
    // Seek to the end to find the footer.
    file_stream_.seekg(0, std::ios::end);
    if (file_stream_.tellg() < static_cast<std::streamoff>(sizeof(uint64_t))) {
        // File is too small to contain even the footer.
        file_stream_.seekg(0); // Reset for other operations
        return;
    }
    file_stream_.seekg(-static_cast<std::streamoff>(sizeof(uint64_t)), std::ios::end);

    BinaryStreamBuffer bsb(file_stream_);
    uint64_t index_start_offset;
    bsb.read(index_start_offset);

    // Seek to the beginning of the index block.
    file_stream_.seekg(index_start_offset);

    // Read the number of entries.
    size_t index_size;
    bsb.read(index_size);

    // Read the index entries.
    sparse_index_.clear();
    for (size_t i = 0; i < index_size; ++i) {
        std::string key = bsb.read_string();
        uint64_t offset;
        bsb.read(offset);
        sparse_index_[key] = offset;
    }

    // Reset stream to beginning for subsequent find operations
    file_stream_.clear();
    file_stream_.seekg(0);
}

} // namespace Storage
} // namespace TissDB
