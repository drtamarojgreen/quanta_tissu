#include "indexer.h"
#include <algorithm>
#include <sstream>
#include <filesystem>
#include "../json/json.h"

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

void Indexer::create_index(const std::vector<std::string>& field_names, bool is_unique, IndexType type) {
    std::string index_name = get_index_name(field_names);
    if (indexes_.count(index_name) || timestamp_indexes_.count(index_name)) {
        return; // Index already exists
    }

    if (type == IndexType::Timestamp) {
        if (field_names.size() != 1) {
            throw std::runtime_error("Timestamp indexes must be on a single field.");
        }
        if (is_unique) {
            throw std::runtime_error("Unique timestamp indexes are not supported.");
        }
        timestamp_indexes_[index_name] = std::make_shared<BTree<int64_t, std::string>>();
    } else {
        indexes_[index_name] = std::make_shared<BTree<std::string, std::string>>();
    }

    index_fields_[index_name] = field_names;
    index_uniqueness_[index_name] = is_unique;
    index_types_[index_name] = type;
}

void Indexer::create_timestamp_index(const std::vector<std::string>& field_names, bool is_unique) {
    std::string index_name = get_index_name(field_names);
    if (timestamp_indexes_.find(index_name) == timestamp_indexes_.end()) {
        timestamp_indexes_[index_name] = std::make_shared<BTree<int64_t, std::string>>();
        index_fields_[index_name] = field_names;
        index_uniqueness_[index_name] = is_unique;
    }
}

bool Indexer::has_index(const std::vector<std::string>& field_names) const {
    std::string index_name = get_index_name(field_names);
    return indexes_.count(index_name) > 0 || timestamp_indexes_.count(index_name) > 0;
}

// Private helper to get a composite key from a document
std::string Indexer::get_composite_key(const std::vector<std::string>& field_names, const Document& doc) const {
    std::stringstream key_ss;
    for (size_t i = 0; i < field_names.size(); ++i) {
        const auto& field_name = field_names[i];
        bool field_found = false;
        for (const auto& elem : doc.elements) {
            if (elem.key == field_name) {
                if (const auto* str_val = std::get_if<std::string>(&elem.value)) {
                    key_ss << *str_val;
                } else if (const auto* num_val = std::get_if<double>(&elem.value)) {
                    key_ss << *num_val;
                } else if (const auto* bool_val = std::get_if<bool>(&elem.value)) {
                    key_ss << (*bool_val ? "true" : "false");
                }
                field_found = true;
                break;
            }
        }
        if (!field_found) {
            return ""; // One of the fields was not in the document, so no key can be generated
        }
        if (i < field_names.size() - 1) {
            key_ss << '\0';
        }
    }
    return key_ss.str();
}

void Indexer::update_indexes(const std::string& document_id, const Document& doc) {
    for (const auto& pair : index_fields_) {
        const std::string& index_name = pair.first;
        const auto& field_names = pair.second;

        if (timestamp_indexes_.count(index_name)) {
            // Handle timestamp index
            if (field_names.size() != 1) continue; // Timestamp indexes are single-field only
            const auto& field_name = field_names[0];
            const TissDB::Value* value = nullptr;
            for (const auto& elem : doc.elements) {
                if (elem.key == field_name) {
                    value = &elem.value;
                    break;
                }
            }

            if (value && std::holds_alternative<TissDB::Timestamp>(*value)) {
                int64_t key = std::get<TissDB::Timestamp>(*value).microseconds_since_epoch_utc;
                auto& btree = timestamp_indexes_[index_name];
                auto existing_json_str_opt = btree->find(key);
                bool is_unique = index_uniqueness_.count(index_name) && index_uniqueness_.at(index_name);

                Json::JsonArray doc_ids_array;
                if (existing_json_str_opt.has_value()) {
                    try {
                        doc_ids_array = Json::JsonValue::parse(existing_json_str_opt.value()).as_array();
                    } catch (...) { /* Ignore parse error, start fresh */ }
                }

                bool already_exists = false;
                for (const auto& id_val : doc_ids_array) {
                    if (id_val.as_string() == document_id) {
                        already_exists = true;
                        break;
                    }
                }

                if (already_exists) {
                    continue;
                }

                if (is_unique && !doc_ids_array.empty()) {
                    throw std::runtime_error("Uniqueness constraint violated for timestamp index '" + index_name + "'");
                }

                doc_ids_array.push_back(Json::JsonValue(document_id));
                btree->insert(key, Json::JsonValue(doc_ids_array).serialize());
            }
        } else if (indexes_.count(index_name)) {
            // Handle string-based composite index
            std::string key = get_composite_key(field_names, doc);
            if (key.empty()) {
                continue; // Skip if document doesn't have all indexed fields
            }

            auto& btree = indexes_[index_name];
            auto existing_json_str_opt = btree->find(key);
            bool is_unique = index_uniqueness_.count(index_name) && index_uniqueness_.at(index_name);

            Json::JsonArray doc_ids_array;
            if (existing_json_str_opt.has_value()) {
                try {
                    doc_ids_array = Json::JsonValue::parse(existing_json_str_opt.value()).as_array();
                } catch (...) { /* Ignore parse error, start fresh */ }
            }

            bool already_exists = false;
            for (const auto& id_val : doc_ids_array) {
                if (id_val.as_string() == document_id) {
                    already_exists = true;
                    break;
                }
            }

            if (already_exists) {
                continue;
            }

            if (is_unique && !doc_ids_array.empty()) {
                throw std::runtime_error("Uniqueness constraint violated for index '" + index_name + "' with key '" + key + "'");
            }

            doc_ids_array.push_back(Json::JsonValue(document_id));
            btree->insert(key, Json::JsonValue(doc_ids_array).serialize());
        }
    }
}

void Indexer::remove_from_indexes(const std::string& document_id, const Document& doc) {
    for (const auto& pair : index_fields_) {
        const std::string& index_name = pair.first;
        const auto& field_names = pair.second;

        if (timestamp_indexes_.count(index_name)) {
            // Handle timestamp index
             if (field_names.size() != 1) continue;
            const auto& field_name = field_names[0];
            const TissDB::Value* value = nullptr;
            for (const auto& elem : doc.elements) {
                if (elem.key == field_name) {
                    value = &elem.value;
                    break;
                }
            }
            if (value && std::holds_alternative<TissDB::Timestamp>(*value)) {
                int64_t key = std::get<TissDB::Timestamp>(*value).microseconds_since_epoch_utc;
                auto& btree = timestamp_indexes_[index_name];
                auto existing_json_str_opt = btree->find(key);

                if (existing_json_str_opt.has_value()) {
                    Json::JsonArray new_doc_ids_array;
                    bool found = false;
                    try {
                        Json::JsonArray old_doc_ids_array = Json::JsonValue::parse(existing_json_str_opt.value()).as_array();
                        for (const auto& id_val : old_doc_ids_array) {
                            if (id_val.as_string() != document_id) {
                                new_doc_ids_array.push_back(id_val);
                            } else {
                                found = true;
                            }
                        }
                    } catch (...) { continue; /* Ignore malformed JSON */ }

                    if (found) {
                        if (new_doc_ids_array.empty()) {
                            btree->erase(key);
                        } else {
                            btree->insert(key, Json::JsonValue(new_doc_ids_array).serialize());
                        }
                    }
                }
            }

        } else if (indexes_.count(index_name)) {
            // Handle string-based composite index
            std::string key = get_composite_key(field_names, doc);
            if (key.empty()) {
                continue;
            }

            auto& btree = indexes_[index_name];
            auto existing_json_str_opt = btree->find(key);

            if (existing_json_str_opt.has_value()) {
                Json::JsonArray new_doc_ids_array;
                bool found = false;
                try {
                    Json::JsonArray old_doc_ids_array = Json::JsonValue::parse(existing_json_str_opt.value()).as_array();
                    for (const auto& id_val : old_doc_ids_array) {
                        if (id_val.as_string() != document_id) {
                            new_doc_ids_array.push_back(id_val);
                        } else {
                            found = true;
                        }
                    }
                } catch (...) { continue; /* Ignore malformed JSON */ }

                if (found) {
                    if (new_doc_ids_array.empty()) {
                        btree->erase(key);
                    } else {
                        btree->insert(key, Json::JsonValue(new_doc_ids_array).serialize());
                    }
                }
            }
        }
    }
}

std::vector<std::string> Indexer::find_by_index(const std::string& index_name, const std::string& value) const {
    auto it = indexes_.find(index_name);
    if (it != indexes_.end()) {
        const auto& btree = it->second;
        auto json_str_opt = btree->find(value);

        if (json_str_opt.has_value()) {
            std::vector<std::string> doc_ids;
            try {
                Json::JsonArray ids_array = Json::JsonValue::parse(json_str_opt.value()).as_array();
                for (const auto& id_val : ids_array) {
                    doc_ids.push_back(id_val.as_string());
                }
            } catch (...) { /* Ignore malformed JSON */ }
            return doc_ids;
        }
    } else {
        auto ts_it = timestamp_indexes_.find(index_name);
        if (ts_it != timestamp_indexes_.end()) {
            // This overload is for string values, but we might get a numeric string for a timestamp.
            // For simplicity, we'll assume direct int64_t lookups will be used for timestamps.
            // A more robust implementation would try to parse the string `value` to an int64_t.
        }
    }


    return {};
}

std::vector<std::string> find_by_index(const std::string& index_name, int64_t value) const {
    auto it = timestamp_indexes_.find(index_name);
    if (it != timestamp_indexes_.end()) {
        const auto& btree = it->second;
        auto json_str_opt = btree->find(value);

        if (json_str_opt.has_value()) {
            std::vector<std::string> doc_ids;
            try {
                Json::JsonArray ids_array = Json::JsonValue::parse(json_str_opt.value()).as_array();
                for (const auto& id_val : ids_array) {
                    doc_ids.push_back(id_val.as_string());
                }
            } catch (...) { /* Ignore malformed JSON */ }
            return doc_ids;
        }
    }
    return {};
}

std::vector<std::string> Indexer::find_by_index(const std::vector<std::string>& field_names, const std::vector<std::string>& values) const {
    if (field_names.size() != values.size()) {
        return {}; // Or throw an error
    }

    std::string index_name = get_index_name(field_names);
    if (indexes_.count(index_name)) {
        auto it = indexes_.find(index_name);
        std::stringstream key_ss;
        for (size_t i = 0; i < values.size(); ++i) {
            key_ss << values[i];
            if (i < values.size() - 1) {
                key_ss << ' ';
            }
        }
        std::string key = key_ss.str();

        const auto& btree = it->second;
        auto json_str_opt = btree->find(key);

        if (json_str_opt.has_value()) {
            std::vector<std::string> doc_ids;
            try {
                Json::JsonArray ids_array = Json::JsonValue::parse(json_str_opt.value()).as_array();
                for (const auto& id_val : ids_array) {
                    doc_ids.push_back(id_val.as_string());
                }
            } catch (...) { /* Ignore malformed JSON */ }
            return doc_ids;
        }
    } else if (timestamp_indexes_.count(index_name)) {
        // This is a bit of a simplification. A real implementation would need to
        // handle type conversion from string in `values` to int64_t.
    }

    return {};
}

std::vector<std::string> Indexer::find_by_index(const std::vector<std::string>& field_names) const {
    std::string index_name = get_index_name(field_names);
    auto it = indexes_.find(index_name);
    if (it == indexes_.end()) {
        // Also check timestamp indexes
        auto ts_it = timestamp_indexes_.find(index_name);
        if (ts_it == timestamp_indexes_.end()) {
            return {};
        }
        // This function returns all doc IDs, so we need to iterate the timestamp tree
        std::vector<std::string> all_doc_ids;
        const auto& btree = ts_it->second;
        btree->foreach([&all_doc_ids](const int64_t& /*key*/, const std::string& value) {
            try {
                Json::JsonArray ids_array = Json::JsonValue::parse(value).as_array();
                for (const auto& id_val : ids_array) {
                    all_doc_ids.push_back(id_val.as_string());
                }
            } catch (...) { /* Ignore malformed JSON */ }
        });
        return all_doc_ids;
    }

    std::vector<std::string> all_doc_ids;
    const auto& btree = it->second;

    btree->foreach([&all_doc_ids](const std::string& /*key*/, const std::string& value) {
        try {
            Json::JsonArray ids_array = Json::JsonValue::parse(value).as_array();
            for (const auto& id_val : ids_array) {
                all_doc_ids.push_back(id_val.as_string());
            }
        } catch (...) { /* Ignore malformed JSON */ }
    });

    return all_doc_ids;
}

std::vector<std::string> Indexer::find_by_timestamp_range(const std::string& index_name, int64_t start_key, int64_t end_key) const {
    auto it = timestamp_indexes_.find(index_name);
    if (it == timestamp_indexes_.end()) {
        return {};
    }

    const auto& btree = it->second;
    auto results = btree->find_range(start_key, end_key);

    std::vector<std::string> doc_ids;
    for (const auto& pair : results) {
        try {
            Json::JsonArray ids_array = Json::JsonValue::parse(pair.second).as_array();
            for (const auto& id_val : ids_array) {
                doc_ids.push_back(id_val.as_string());
            }
        } catch (...) { /* Ignore malformed JSON */ }
    }
    // Remove duplicates
    std::sort(doc_ids.begin(), doc_ids.end());
    doc_ids.erase(std::unique(doc_ids.begin(), doc_ids.end()), doc_ids.end());

    return doc_ids;
}

void Indexer::save_indexes(const std::string& data_dir) {
    if (!std::filesystem::exists(data_dir)) {
        std::filesystem::create_directories(data_dir);
    }

    // Save the index metadata
    Json::JsonObject meta_obj;
    Json::JsonObject fields_obj;
    Json::JsonObject unique_obj;

    for (const auto& pair : index_fields_) {
        Json::JsonArray fields_array;
        for (const auto& field : pair.second) {
            fields_array.push_back(Json::JsonValue(field));
        }
        fields_obj[pair.first] = Json::JsonValue(fields_array);
    }

    for (const auto& pair : index_uniqueness_) {
        unique_obj[pair.first] = Json::JsonValue(pair.second);
    }

    meta_obj["fields"] = Json::JsonValue(fields_obj);
    meta_obj["unique"] = Json::JsonValue(unique_obj);

    std::string meta_path = data_dir + "/indexes.meta";
    std::ofstream meta_ofs(meta_path);
    meta_ofs << Json::JsonValue(meta_obj).serialize();
    meta_ofs.close();

    // Save the B-Tree data
    for (const auto& pair : indexes_) {
        std::string bpt_path = data_dir + "/" + pair.first + ".bpt";
        std::ofstream ofs(bpt_path, std::ios::binary);
        pair.second->dump(ofs);
    }
}

std::vector<std::vector<std::string>> Indexer::get_available_indexes() const {
    std::vector<std::vector<std::string>> all_indexes;
    for (const auto& pair : index_fields_) {
        all_indexes.push_back(pair.second);
    }
    return all_indexes;
}


void Indexer::load_indexes(const std::string& data_dir) {
    indexes_.clear();
    index_fields_.clear();

    std::string meta_path = data_dir + "/indexes.meta";
    if (!std::filesystem::exists(meta_path)) {
        return; // No indexes to load
    }

    // Load index metadata
    std::ifstream meta_ifs(meta_path);
    if (meta_ifs.is_open()) {
        std::string meta_content((std::istreambuf_iterator<char>(meta_ifs)), std::istreambuf_iterator<char>());
        try {
            auto meta_json = Json::JsonValue::parse(meta_content).as_object();
            if (meta_json.count("fields")) {
                auto fields_obj = meta_json.at("fields").as_object();
                for (const auto& pair : fields_obj) {
                    std::vector<std::string> fields;
                    for (const auto& field_val : pair.second.as_array()) {
                        fields.push_back(field_val.as_string());
                    }
                    index_fields_[pair.first] = fields;
                }
            }
            if (meta_json.count("unique")) {
                auto unique_obj = meta_json.at("unique").as_object();
                for (const auto& pair : unique_obj) {
                    index_uniqueness_[pair.first] = pair.second.as_bool();
                }
            }
        } catch (...) {
            // Handle parsing error if necessary, maybe log it
            return;
        }
    }

    // Load B-Tree data
    for (const auto& pair : index_fields_) {
        std::string index_name = pair.first;
        std::string bpt_path = data_dir + "/" + index_name + ".bpt";
        if (std::filesystem::exists(bpt_path)) {
            try {
                auto btree = std::make_shared<BTree<std::string, std::string>>();
                std::ifstream ifs(bpt_path, std::ios::binary);
                btree->load(ifs);
                indexes_[index_name] = btree;
            } catch (...) {
                // Handle B-Tree deserialization error, maybe log it
            }
        }
    }
}


} // namespace Storage
} // namespace TissDB