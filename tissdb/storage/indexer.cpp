#include "indexer.h"
#include "btree.h"
#include <algorithm>

namespace TissDB {
namespace Storage {

Indexer::Indexer() = default;

void Indexer::create_index(const std::string& field_name) {
    if (indexes.find(field_name) == indexes.end()) {
        indexes[field_name] = std::make_unique<BTree>();
    }
}

bool Indexer::has_index(const std::string& field_name) const {
    return indexes.count(field_name) > 0;
}

void Indexer::update_indexes(const std::string& document_id, const Document& doc) {
    for (const auto& pair : indexes) {
        for (const auto& elem : doc.elements) {
            if (elem.key == pair.first) {
                if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                    pair.second->insert(*str_val, document_id);
                }
            }
        }
    }
}

void Indexer::remove_from_indexes(const std::string& document_id, const Document& doc) {
    for (const auto& pair : indexes) {
        for (const auto& elem : doc.elements) {
            if (elem.key == pair.first) {
                if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                    pair.second->remove(*str_val);
                }
            }
        }
    }
}

std::vector<std::string> Indexer::find_by_index(const std::string& field_name, const std::string& value) const {
    auto it = indexes.find(field_name);
    if (it != indexes.end()) {
        return it->second->find(value);
    }
    return {};
}

void Indexer::save_indexes(const std::string& data_dir) {
    for (const auto& pair : indexes) {
        std::ofstream ofs(data_dir + "/" + pair.first + ".idx", std::ios::binary);
        pair.second->serialize(ofs);
    }
}


void Indexer::load_indexes(const std::string& data_dir) {
    // This implementation is Windows-specific.
    // A portable implementation would require a different approach.
#ifdef _WIN32
    #include <windows.h>
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((data_dir + "/*.idx").c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string index_name = findFileData.cFileName;
            index_name = index_name.substr(0, index_name.find_last_of("."));
            indexes[index_name] = std::make_unique<BTree>();
            std::ifstream ifs(data_dir + "/" + findFileData.cFileName, std::ios::binary);
            indexes[index_name]->deserialize(ifs);
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#endif
}


} // namespace Storage
} // namespace TissDB
