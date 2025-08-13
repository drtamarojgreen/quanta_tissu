#include "sstable.h"
#include "../common/serialization.h"
#include <iostream>
#include <chrono>

namespace TissDB {
namespace Storage {

namespace {
// Helper to write a length-prefixed block of data.
void write_prefixed(std::ostream& os, const char* data, size_t len) {
    os.write(reinterpret_cast<const char*>(&len), sizeof(len));
    os.write(data, len);
}
} // anonymous namespace

// --- SSTable Public Methods ---

SSTable::SSTable(const std::string& path) : file_path_(path) {
    // In a real implementation, this would open the file and load the index.
    // For now, we'll open the file on-demand in find().
    // load_index();
}

std::optional<std::vector<uint8_t>> SSTable::find(const std::string& key) {
    // This is a simplified `find` that performs a linear scan of the entire file.
    // A real implementation would use the sparse_index_ to seek to the correct
    // block and scan only a small portion of the file.
    std::ifstream sst_file(file_path_, std::ios::binary);
    if (!sst_file.is_open()) {
        // This could be an error or just mean the file doesn't exist.
        return std::nullopt;
    }

    size_t key_len;
    while (sst_file.read(reinterpret_cast<char*>(&key_len), sizeof(key_len))) {
        std::string current_key(key_len, '\0');
        sst_file.read(&current_key[0], key_len);

        size_t val_len;
        sst_file.read(reinterpret_cast<char*>(&val_len), sizeof(val_len));

        if (current_key == key) {
            if (val_len == static_cast<size_t>(-1)) { // Tombstone marker
                return std::vector<uint8_t>(); // Empty vector for tombstone
            }
            std::vector<uint8_t> value(val_len);
            sst_file.read(reinterpret_cast<char*>(value.data()), val_len);
            return value;
        } else {
            // Key doesn't match, so skip over the value bytes to get to the next key.
            if (val_len != static_cast<size_t>(-1)) {
                 sst_file.seekg(val_len, std::ios_base::cur);
            }
        }
    }
    return std::nullopt; // Key not found
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

    // The memtable's map is already sorted by key, which is exactly what we need.
    const auto& data = memtable.get_all();
    for (const auto& pair : data) {
        // Write key (length-prefixed)
        write_prefixed(sst_file, pair.first.data(), pair.first.size());

        // Write value (length-prefixed)
        if (pair.second) { // If it's a document pointer
            std::vector<uint8_t> value_bytes = TissDB::serialize(*(pair.second));
            write_prefixed(sst_file, reinterpret_cast<const char*>(value_bytes.data()), value_bytes.size());
        } else { // If it's a tombstone (nullptr)
            size_t tombstone_marker = static_cast<size_t>(-1);
            sst_file.write(reinterpret_cast<const char*>(&tombstone_marker), sizeof(tombstone_marker));
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
