#include "database_manager.h"
#include "../json/json.h"
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <vector>

namespace TissDB {
namespace Storage {

namespace fs = std::filesystem;

// Forward declaration for helper functions
void load_manifest(const std::string& manifest_path, std::map<std::string, std::unique_ptr<LSMTree>>& databases, const std::string& base_data_path);
void save_manifest(const std::string& manifest_path, const std::map<std::string, std::unique_ptr<LSMTree>>& databases);

DatabaseManager::DatabaseManager(const std::string& base_path) : base_data_path_(base_path) {
    if (!fs::exists(base_data_path_)) {
        fs::create_directories(base_data_path_);
    }
    // Load existing databases from the manifest file on startup
    load_manifest((fs::path(base_data_path_) / "manifest.json").string(), databases_, base_data_path_);
}

DatabaseManager::~DatabaseManager() = default;

bool DatabaseManager::create_database(const std::string& db_name) {
    if (database_exists(db_name)) {
        return false; // Database already exists.
    }

    std::string db_path = (fs::path(base_data_path_) / db_name).string();
    if (!fs::exists(db_path)) {
        fs::create_directory(db_path);
    }

    databases_[db_name] = std::make_unique<LSMTree>(db_path);

    // Update the manifest on disk
    save_manifest((fs::path(base_data_path_) / "manifest.json").string(), databases_);
    return true;
}

void DatabaseManager::delete_database(const std::string& db_name) {
    if (!database_exists(db_name)) {
        return; // It's okay to try to delete a non-existent database.
    }

    databases_.erase(db_name);

    std::string db_path = (fs::path(base_data_path_) / db_name).string();
    if (fs::exists(db_path)) {
        fs::remove_all(db_path);
    }

    // Update the manifest on disk
    save_manifest((fs::path(base_data_path_) / "manifest.json").string(), databases_);
}

LSMTree& DatabaseManager::get_database(const std::string& db_name) {
    auto it = databases_.find(db_name);
    if (it == databases_.end()) {
        throw std::runtime_error("Database '" + db_name + "' not found.");
    }
    return *it->second;
}

bool DatabaseManager::database_exists(const std::string& db_name) const {
    return databases_.count(db_name) > 0;
}

std::vector<std::string> DatabaseManager::list_databases() const {
    std::vector<std::string> names;
    names.reserve(databases_.size());
    for (const auto& pair : databases_) {
        names.push_back(pair.first);
    }
    return names;
}

void DatabaseManager::shutdown() {
    for (auto const& [name, db] : databases_) {
        db->shutdown();
    }
}


// --- Helper Function Implementations ---

void load_manifest(const std::string& manifest_path, std::map<std::string, std::unique_ptr<LSMTree>>& databases, const std::string& base_data_path) {
    if (!fs::exists(manifest_path)) {
        return; // No manifest to load
    }

    std::ifstream manifest_file(manifest_path);
    if (!manifest_file.is_open()) {
        throw std::runtime_error("Could not open manifest file for reading: " + manifest_path);
    }

    std::string content((std::istreambuf_iterator<char>(manifest_file)), std::istreambuf_iterator<char>());
    manifest_file.close();

    if (content.empty()) {
        return; // Manifest is empty
    }

    try {
        Json::JsonValue parsed = Json::JsonValue::parse(content);
        const auto& dbs_array = parsed.as_object().at("databases").as_array();
        for (const auto& db_val : dbs_array) {
            std::string db_name = db_val.as_string();
            if (databases.find(db_name) == databases.end()) {
                 std::string db_path = (fs::path(base_data_path) / db_name).string();
                 databases[db_name] = std::make_unique<LSMTree>(db_path);
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse manifest file: " + std::string(e.what()));
    }
}

void save_manifest(const std::string& manifest_path, const std::map<std::string, std::unique_ptr<LSMTree>>& databases) {
    Json::JsonObject manifest_obj;
    Json::JsonArray db_array;
    for (const auto& pair : databases) {
        db_array.push_back(Json::JsonValue(pair.first));
    }
    manifest_obj["databases"] = Json::JsonValue(db_array);

    std::ofstream manifest_file(manifest_path, std::ios::trunc);
    if (!manifest_file.is_open()) {
        throw std::runtime_error("Could not open manifest file for writing: " + manifest_path);
    }
    manifest_file << Json::JsonValue(manifest_obj).serialize();
    manifest_file.close();
}

} // namespace Storage
} // namespace TissDB
