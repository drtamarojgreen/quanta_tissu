#include "indexer.h"
#include <algorithm>
#include <fstream>
#include <sstream>

// This is a placeholder for the actual B++ tree library
#include "bpp_tree.h"

namespace TissDB {
namespace Storage {

Indexer::Indexer() = default;

std::string Indexer::get_index_name(const std::vector<std::string>& field_names) const {
    std::stringstream ss;
    for (size_t i = 0; i < field_names.size(); ++i) {
        ss << field_names[i];
        if (i < field_names.size() - 1) {
            ss << "_";
        }
    }
    return ss.str();
}

void Indexer::create_index(const std::vector<std::string>& field_names) {
    std::string index_name = get_index_name(field_names);
    if (indexes_.find(index_name) == indexes_.end()) {
        indexes_[index_name] = std::make_unique<bpp::btree<std::string, std::string>>();
        index_fields_[index_name] = field_names;
    }
}

bool Indexer::has_index(const std::vector<std::string>& field_names) const {
    return indexes_.count(get_index_name(field_names)) > 0;
}

void Indexer::update_indexes(const std::string& document_id, const Document& doc) {
    for (const auto& pair : indexes_) {
        const auto& field_names = index_fields_.at(pair.first);
        std::stringstream key_ss;
        bool all_fields_present = true;
        for (const auto& field_name : field_names) {
            bool field_found = false;
            for (const auto& elem : doc.elements) {
                if (elem.key == field_name) {
                    if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                        key_ss << *str_val;
                        field_found = true;
                        break;
                    }
                }
            }
            if (!field_found) {
                all_fields_present = false;
                break;
            }
        }

        if (all_fields_present) {
            pair.second->insert(key_ss.str(), document_id);
        }
    }
}

void Indexer::remove_from_indexes(const std::string& document_id, const Document& doc) {
    for (const auto& pair : indexes_) {
        const auto& field_names = index_fields_.at(pair.first);
        std::stringstream key_ss;
        bool all_fields_present = true;
        for (const auto& field_name : field_names) {
            bool field_found = false;
            for (const auto& elem : doc.elements) {
                if (elem.key == field_name) {
                    if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                        key_ss << *str_val;
                        field_found = true;
                        break;
                    }
                }
            }
            if (!field_found) {
                all_fields_present = false;
                break;
            }
        }

        if (all_fields_present) {
            pair.second->erase(key_ss.str());
        }
    }
}

std::vector<std::string> Indexer::find_by_index(const std::string& field_name, const std::string& value) const {
    // This method now only supports single-field indexes for simplicity.
    // A more advanced implementation would be needed to support compound index lookups.
    auto it = indexes_.find(field_name);
    if (it != indexes_.end()) {
        auto result = it->second->find(value);
        if (result) {
            return {*result};
        }
    }
    return {};
}

void Indexer::save_indexes(const std::string& data_dir) {
    for (const auto& pair : indexes_) {
        std::ofstream ofs(data_dir + "/" + pair.first + ".idx", std::ios::binary);
        pair.second->dump(ofs);
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
            indexes_[index_name] = std::make_unique<bpp::btree<std::string, std::string>>();
            std::ifstream ifs(data_dir + "/" + findFileData.cFileName, std::ios::binary);
            indexes_[index_name]->load(ifs);
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#endif
}


} // namespace Storage
} // namespace TissDB