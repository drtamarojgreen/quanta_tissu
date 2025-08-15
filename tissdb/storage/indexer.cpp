#include "indexer.h"
#include <algorithm>
#include <sstream>

namespace TissDB {
namespace Storage {

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
        indexes_[index_name] = {};
        index_fields_[index_name] = field_names;
    }
}

bool Indexer::has_index(const std::vector<std::string>& field_names) const {
    return indexes_.count(get_index_name(field_names)) > 0;
}

void Indexer::update_indexes(const std::string& document_id, const Document& doc) {
    for (const auto& pair : index_fields_) {
        const std::string& index_name = pair.first;
        const auto& field_names = pair.second;

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
            indexes_[index_name][key_ss.str()] = document_id;
        }
    }
}

void Indexer::remove_from_indexes(const std::string& document_id, const Document& doc) {
    // This is a simplified implementation for the stub.
    // A real implementation would need to handle document removal more robustly.
    (void)document_id; // Unused
    for (const auto& pair : index_fields_) {
        const std::string& index_name = pair.first;
        const auto& field_names = pair.second;

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
            auto it = indexes_.find(index_name);
            if (it != indexes_.end()) {
                it->second.erase(key_ss.str());
            }
        }
    }
}

std::vector<std::string> Indexer::find_by_index(const std::string& field_name, const std::string& value) const {
    // This stub only supports single-field indexes.
    auto it = indexes_.find(field_name);
    if (it != indexes_.end()) {
        auto doc_it = it->second.find(value);
        if (doc_it != it->second.end()) {
            return {doc_it->second};
        }
    }
    return {};
}

} // namespace Storage
} // namespace TissDB