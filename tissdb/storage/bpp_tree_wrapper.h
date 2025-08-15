#pragma once

#include "BPTree.h"
#include <memory>
#include <optional>
#include <fstream>
#include <string>

// NOTE: The underlying B+ Tree implementation is from https://github.com/SirLYC/BPTree.
// The repository does not have a license, so it is used here for demonstration purposes only.
// For production use, a library with a proper license should be used.

namespace bpp {

template<typename Key, typename Value>
class btree {
public:
    btree() : tree_(std::make_unique<BPTree<Key, Value>>(10)) {} // Default order 10

    void insert(const Key& key, const Value& value) {
        tree_->put(key, value);
    }

    std::optional<Value> find(const Key& key) {
        Value* result = tree_->get(key);
        if (result) {
            return *result;
        }
        return std::nullopt;
    }

    void erase(const Key& key) {
        Key non_const_key = key;
        tree_->remove(non_const_key);
    }

    void dump(std::ostream& os) {
        // The SirLYC/BPTree library serializes to a file path, not a stream.
        // We will simulate this by writing to a temporary file and then copying to the stream.
        std::string temp_path = "temp_bpp_tree.bin";
        tree_->serialize(temp_path);

        std::ifstream ifs(temp_path, std::ios::binary);
        os << ifs.rdbuf();
        std::remove(temp_path.c_str());
    }

    void load(std::istream& is) {
        // The SirLYC/BPTree library deserializes from a file path, not a stream.
        // We will simulate this by writing the stream to a temporary file and then loading from it.
        std::string temp_path = "temp_bpp_tree.bin";
        std::ofstream ofs(temp_path, std::ios::binary);
        ofs << is.rdbuf();
        ofs.close();

        DefaultCompare<Key> comp;
        tree_ = BPTree<Key, Value>::deserialize(temp_path, comp);
        std::remove(temp_path.c_str());
    }

private:
    std::unique_ptr<BPTree<Key, Value>> tree_;
};

} // namespace bpp
