#include "lsm_tree.h"
#include "transaction_manager.h"
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace TissDB {
namespace Storage {

LSMTree::LSMTree(const std::string& data_dir) : data_directory_(data_dir) {
    std::filesystem::create_directories(data_directory_);
    transaction_manager_ = new Transactions::TransactionManager();

    // Load existing collections from the data directory on startup.
    for (const auto& entry : std::filesystem::directory_iterator(data_directory_)) {
        if (entry.is_directory()) {
            collections_[entry.path().filename().string()] = std::make_unique<Collection>(entry.path().string());
        }
    }
}

LSMTree::~LSMTree() {
    shutdown();
    delete transaction_manager_;
}

void LSMTree::create_collection(const std::string& name, const TissDB::Schema& schema) {
    if (collections_.count(name)) {
        throw std::runtime_error("Collection '" + name + "' already exists.");
    }
    std::string collection_path = data_directory_ + "/" + name;
    auto collection = std::make_unique<Collection>(collection_path);
    collection->set_schema(schema);
    collections_[name] = std::move(collection);
}

void LSMTree::delete_collection(const std::string& name) {
    if (!collections_.count(name)) {
        throw std::runtime_error("Collection '" + name + "' does not exist.");
    }
    collections_[name]->shutdown(); // Ensure data is flushed and indexes saved
    collections_.erase(name);
    std::filesystem::remove_all(data_directory_ + "/" + name);
}

std::vector<std::string> LSMTree::list_collections() const {
    std::vector<std::string> names;
    for (const auto& pair : collections_) {
        names.push_back(pair.first);
    }
    return names;
}

void LSMTree::put(const std::string& collection_name, const std::string& key, const Document& doc) {
    get_collection(collection_name).put(key, doc);
}

std::optional<Document> LSMTree::get(const std::string& collection_name, const std::string& key) {
    return get_collection(collection_name).get(key);
}

void LSMTree::del(const std::string& collection_name, const std::string& key) {
    get_collection(collection_name).del(key);
}

std::vector<Document> LSMTree::scan(const std::string& collection_name) {
    return get_collection(collection_name).scan();
}

void LSMTree::create_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
    get_collection(collection_name).create_index(field_names);
}

std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) {
    return get_collection(collection_name).find_by_index(field_name, value);
}

std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values) {
    return get_collection(collection_name).find_by_index(field_names, values);
}

std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
    return get_collection(collection_name).find_by_index(field_names);
}

bool LSMTree::has_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
    return get_collection(collection_name).has_index(field_names);
}

int LSMTree::begin_transaction() {
    return transaction_manager_->begin_transaction();
}

void LSMTree::commit_transaction(int transaction_id) {
    // Apply all operations in the transaction to the storage engine
    const auto& operations = transaction_manager_->get_transactions().at(transaction_id)->get_operations();
    for (const auto& op : operations) {
        if (op.type == TissDB::Transactions::OperationType::PUT) {
            get_collection(op.collection_name).put(op.key, op.doc);
        } else if (op.type == TissDB::Transactions::OperationType::DELETE) {
            get_collection(op.collection_name).del(op.key);
        }
    }
    transaction_manager_->commit_transaction(transaction_id);
}

void LSMTree::rollback_transaction(int transaction_id) {
    transaction_manager_->rollback_transaction(transaction_id);
}

void LSMTree::shutdown() {
    for (const auto& pair : collections_) {
        pair.second->shutdown();
    }
}

Collection& LSMTree::get_collection(const std::string& name) {
    auto it = collections_.find(name);
    if (it == collections_.end()) {
        throw std::runtime_error("Collection '" + name + "' not found.");
    }
    return *(it->second);
}

} // namespace Storage
} // namespace TissDB