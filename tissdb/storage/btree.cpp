
#include "btree.h"

namespace TissDB {
namespace Storage {

BTree::BTree() {
    root = std::make_unique<BTreeNode>();
    root->is_leaf = true;
    t = 2; // Minimum degree
}

void BTree::insert(const std::string& key, const std::string& doc_id) {
    if (root->keys.size() == (2 * t - 1)) {
        auto new_root = std::make_unique<BTreeNode>();
        new_root->is_leaf = false;
        new_root->children.push_back(std::move(root));
        root = std::move(new_root);
        split_child(root.get(), 0);
    }
    insert_non_full(root.get(), key, doc_id);
}

void BTree::insert_non_full(BTreeNode* node, const std::string& key, const std::string& doc_id) {
    int i = node->keys.size() - 1;
    if (node->is_leaf) {
        node->keys.push_back("");
        node->doc_ids.push_back("");
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            node->doc_ids[i + 1] = node->doc_ids[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->doc_ids[i + 1] = doc_id;
    } else {
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        if (node->children[i]->keys.size() == (2 * t - 1)) {
            split_child(node, i);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insert_non_full(node->children[i].get(), key, doc_id);
    }
}

void BTree::split_child(BTreeNode* parent, int i) {
    auto child = parent->children[i].get();
    auto new_child = std::make_unique<BTreeNode>();
    new_child->is_leaf = child->is_leaf;

    parent->keys.insert(parent->keys.begin() + i, child->keys[t - 1]);
    parent->children.insert(parent->children.begin() + i + 1, std::move(new_child));

    parent->children[i + 1]->keys.assign(child->keys.begin() + t, child->keys.end());
    child->keys.resize(t - 1);

    if (child->is_leaf) {
        parent->children[i + 1]->doc_ids.assign(child->doc_ids.begin() + t, child->doc_ids.end());
        child->doc_ids.resize(t - 1);
    }
}

BTreeNode* BTree::search(BTreeNode* node, const std::string& key) {
    int i = 0;
    while (i < node->keys.size() && key > node->keys[i]) {
        i++;
    }

    if (i < node->keys.size() && key == node->keys[i]) {
        return node;
    }

    if (node->is_leaf) {
        return nullptr;
    }

    return search(node->children[i].get(), key);
}

std::vector<std::string> BTree::find(const std::string& key) {
    auto node = search(root.get(), key);
    if (node) {
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }
        if (i < node->keys.size() && key == node->keys[i]) {
            return {node->doc_ids[i]};
        }
    }
    return {};
}



void BTree::remove(const std::string& key) {
    // Simplified remove: only removes from the leaf node if found.
    // A full B-Tree deletion algorithm is more complex and involves
    // handling underflow, merging, and borrowing from siblings.
    BTreeNode* node = root.get();
    while (node) {
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }

        if (i < node->keys.size() && key == node->keys[i]) {
            // Found the key in this node, remove it.
            node->keys.erase(node->keys.begin() + i);
            if (node->is_leaf) {
                node->doc_ids.erase(node->doc_ids.begin() + i);
            }
            return;
        }

        if (node->is_leaf) {
            // Key not found in leaf node.
            return;
        }
        node = node->children[i].get();
    }
}


void serialize_node(std::ostream& os, BTreeNode* node) {
    if (!node) return;

    os.write(reinterpret_cast<const char*>(&node->is_leaf), sizeof(node->is_leaf));

    size_t num_keys = node->keys.size();
    os.write(reinterpret_cast<const char*>(&num_keys), sizeof(num_keys));
    for (const auto& key : node->keys) {
        size_t key_size = key.size();
        os.write(reinterpret_cast<const char*>(&key_size), sizeof(key_size));
        os.write(key.c_str(), key_size);
    }

    if (node->is_leaf) {
        size_t num_doc_ids = node->doc_ids.size();
        os.write(reinterpret_cast<const char*>(&num_doc_ids), sizeof(num_doc_ids));
        for (const auto& doc_id : node->doc_ids) {
            size_t doc_id_size = doc_id.size();
            os.write(reinterpret_cast<const char*>(&doc_id_size), sizeof(doc_id_size));
            os.write(doc_id.c_str(), doc_id_size);
        }
    } else {
        size_t num_children = node->children.size();
        os.write(reinterpret_cast<const char*>(&num_children), sizeof(num_children));
        for (const auto& child : node->children) {
            serialize_node(os, child.get());
        }
    }
}

void BTree::serialize(std::ostream& os) {
    serialize_node(os, root.get());
}

std::unique_ptr<BTreeNode> deserialize_node(std::istream& is) {
    auto node = std::make_unique<BTreeNode>();

    is.read(reinterpret_cast<char*>(&node->is_leaf), sizeof(node->is_leaf));

    size_t num_keys;
    is.read(reinterpret_cast<char*>(&num_keys), sizeof(num_keys));
    node->keys.resize(num_keys);
    for (size_t i = 0; i < num_keys; ++i) {
        size_t key_size;
        is.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        node->keys[i].resize(key_size);
        is.read(&node->keys[i][0], key_size);
    }

    if (node->is_leaf) {
        size_t num_doc_ids;
        is.read(reinterpret_cast<char*>(&num_doc_ids), sizeof(num_doc_ids));
        node->doc_ids.resize(num_doc_ids);
        for (size_t i = 0; i < num_doc_ids; ++i) {
            size_t doc_id_size;
            is.read(reinterpret_cast<char*>(&doc_id_size), sizeof(doc_id_size));
            node->doc_ids[i].resize(doc_id_size);
            is.read(&node->doc_ids[i][0], doc_id_size);
        }
    } else {
        size_t num_children;
        is.read(reinterpret_cast<char*>(&num_children), sizeof(num_children));
        node->children.resize(num_children);
        for (size_t i = 0; i < num_children; ++i) {
            node->children[i] = deserialize_node(is);
        }
    }

    return node;
}

void BTree::deserialize(std::istream& is) {
    root = deserialize_node(is);
}


}
}
