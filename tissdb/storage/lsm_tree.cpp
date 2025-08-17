#include "lsm_tree.h"
#include <stdexcept>

namespace TissDB {
namespace Storage {

LSMTree::LSMTree() {}

LSMTree::~LSMTree() {}

bool LSMTree::create_collection(const std::string& name) {
    if (collections_.count(name)) {
        return false; // Collection already exists
    }
    collections_[name] = std::make_unique<Collection>();
    return true;
}

bool LSMTree::drop_collection(const std::string& name) {
    if (!collections_.count(name)) {
        return false; // Collection does not exist
    }
    collections_.erase(name);
    return true;
}

std::vector<std::string> LSMTree::list_collections() const {
    std::vector<std::string> names;
    for (const auto& pair : collections_) {
        names.push_back(pair.first);
    }
    return names;
}

void LSMTree::put(const std::string& collection_name, const std::string& key, const Document& doc) {
    Collection* collection = get_collection(collection_name);
    if (collection) {
        collection->put(key, doc);
    }
}

std::optional<std::shared_ptr<Document>> LSMTree::get(const std::string& collection_name, const std::string& key) {
    Collection* collection = get_collection(collection_name);
    if (collection) {
        return collection->get(key);
    }
    return std::nullopt;
}

void LSMTree::del(const std::string& collection_name, const std::string& key) {
    Collection* collection = get_collection(collection_name);
    if (collection) {
        collection->del(key);
    }
}

std::vector<Document> LSMTree::scan(const std::string& collection_name) {
    Collection* collection = get_collection(collection_name);
    if (collection) {
        return collection->scan();
    }
    return {}; // Return empty vector if collection not found
}

Collection* LSMTree::get_collection(const std::string& name) {
    auto it = collections_.find(name);
    if (it == collections_.end()) {
        return nullptr;
    }
    return it->second.get();
}

} // namespace Storage
} // namespace TissDB