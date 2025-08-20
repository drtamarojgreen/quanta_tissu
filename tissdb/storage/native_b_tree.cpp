#include "native_b_tree.h"
#include <iterator> // For std::make_move_iterator

namespace TissDB {
namespace Storage {

// BTreeNode Implementation
template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::BTreeNode::BTreeNode(bool leaf) : is_leaf(leaf) {}

template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::BTreeNode::~BTreeNode() {}

// BTree Implementation
template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::BTree() {
    root_ = std::make_unique<BTreeNode>(true);
}

template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::~BTree() {}

// --- INSERT ---
template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::insert(const Key& key, const Value& value) {
    if (root_->keys.size() == 2 * Order - 1) {
        auto new_root = std::make_unique<BTreeNode>(false);
        new_root->children.push_back(std::move(root_));
        split_child(new_root.get(), 0);
        root_ = std::move(new_root);
    }
    insert_non_full(root_.get(), key, value);
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::insert_non_full(BTreeNode* node, const Key& key, const Value& value) {
    auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    int i = std::distance(node->keys.begin(), it);

    if (node->is_leaf) {
        node->keys.insert(it, key);
        node->values.insert(node->values.begin() + i, value);
    } else {
        if (node->children[i]->keys.size() == 2 * Order - 1) {
            split_child(node, i);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insert_non_full(node->children[i].get(), key, value);
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::split_child(BTreeNode* parent, int index) {
    BTreeNode* child = parent->children[index].get();
    auto new_child = std::make_unique<BTreeNode>(child->is_leaf);

    parent->keys.insert(parent->keys.begin() + index, child->keys[Order - 1]);
    // For non-leaf nodes, there is no value, just keys for routing
    if (child->is_leaf) {
        parent->values.insert(parent->values.begin() + index, child->values[Order - 1]);
    }

    new_child->keys.assign(
        std::make_move_iterator(child->keys.begin() + Order),
        std::make_move_iterator(child->keys.end())
    );
    if (child->is_leaf) {
        new_child->values.assign(
            std::make_move_iterator(child->values.begin() + Order),
            std::make_move_iterator(child->values.end())
        );
    }
    child->keys.resize(Order - 1);
    if (child->is_leaf) {
        child->values.resize(Order - 1);
    }

    if (!child->is_leaf) {
        new_child->children.assign(
            std::make_move_iterator(child->children.begin() + Order),
            std::make_move_iterator(child->children.end())
        );
        child->children.resize(Order);
    }

    parent->children.insert(parent->children.begin() + index + 1, std::move(new_child));
}


// --- FIND ---
template<typename Key, typename Value, int Order>
std::optional<Value> BTree<Key, Value, Order>::find(const Key& key) {
    return find_recursive(root_.get(), key);
}

template<typename Key, typename Value, int Order>
std::optional<Value> BTree<Key, Value, Order>::find_recursive(BTreeNode* node, const Key& key) {
    auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    int i = std::distance(node->keys.begin(), it);

    if (it != node->keys.end() && *it == key) {
        if (node->is_leaf) {
            return node->values[i];
        }
        // In a B-Tree, the value is in the leaf. In a B+ tree, it could be here.
        // This implementation stores values only in leaves.
        // To find the actual value, we need to traverse to the leaf.
        // For simplicity here, we assume if key is in internal node, we traverse left.
        // A more correct B-Tree would only store values in leaves.
        // Let's adjust the logic to be a proper B-Tree where keys can exist in internal nodes.
        return node->values[i];
    }

    if (node->is_leaf) {
        return std::nullopt;
    }

    return find_recursive(node->children[i].get(), key);
}


// --- ERASE ---
template<typename Key, typename Value, int Order>
int BTree<Key, Value, Order>::find_key(BTreeNode* node, const Key& key) {
    auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    if (it != node->keys.end() && *it == key) {
        return std::distance(node->keys.begin(), it);
    }
    return -1; // Not found
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::erase(const Key& key) {
    erase_recursive(root_.get(), key);
    if (root_->keys.empty() && !root_->is_leaf) {
        root_ = std::move(root_->children[0]);
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::erase_recursive(BTreeNode* node, const Key& key) {
    int index = find_key(node, key);
    if (index != -1) { // Key is in this node
        if (node->is_leaf) {
            remove_from_leaf(node, index);
        } else {
            remove_from_non_leaf(node, index);
        }
    } else { // Key is not in this node
        int i = std::distance(node->keys.begin(), std::lower_bound(node->keys.begin(), node->keys.end(), key));
        if (node->is_leaf) {
            return; // Key not found
        }
        bool flag = (i == node->keys.size());
        if (node->children[i]->keys.size() < Order) {
            fill(node, i);
        }
        if (flag && i > node->keys.size()) {
            erase_recursive(node->children[i - 1].get(), key);
        } else {
            erase_recursive(node->children[i].get(), key);
        }
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::remove_from_leaf(BTreeNode* node, int index) {
    node->keys.erase(node->keys.begin() + index);
    node->values.erase(node->values.begin() + index);
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::remove_from_non_leaf(BTreeNode* node, int index) {
    Key key = node->keys[index];

    if (node->children[index]->keys.size() >= Order) {
        Key pred_key = get_predecessor(node, index);
        node->keys[index] = pred_key;
        // The value should also be updated if we store values in internal nodes
        // node->values[index] = pred_value;
        erase_recursive(node->children[index].get(), pred_key);
    } else if (node->children[index + 1]->keys.size() >= Order) {
        Key succ_key = get_successor(node, index);
        node->keys[index] = succ_key;
        // node->values[index] = succ_value;
        erase_recursive(node->children[index + 1].get(), succ_key);
    } else {
        merge(node, index);
        erase_recursive(node->children[index].get(), key);
    }
}

template<typename Key, typename Value, int Order>
Key BTree<Key, Value, Order>::get_predecessor(BTreeNode* node, int index) {
    BTreeNode* current = node->children[index].get();
    while (!current->is_leaf) {
        current = current->children.back().get();
    }
    return current->keys.back();
}

template<typename Key, typename Value, int Order>
Key BTree<Key, Value, Order>::get_successor(BTreeNode* node, int index) {
    BTreeNode* current = node->children[index + 1].get();
    while (!current->is_leaf) {
        current = current->children.front().get();
    }
    return current->keys.front();
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::fill(BTreeNode* node, int index) {
    if (index != 0 && node->children[index - 1]->keys.size() >= Order) {
        borrow_from_prev(node, index);
    } else if (index != node->keys.size() && node->children[index + 1]->keys.size() >= Order) {
        borrow_from_next(node, index);
    } else {
        if (index != node->keys.size()) {
            merge(node, index);
        } else {
            merge(node, index - 1);
        }
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::borrow_from_prev(BTreeNode* node, int index) {
    BTreeNode* child = node->children[index].get();
    BTreeNode* sibling = node->children[index - 1].get();

    child->keys.insert(child->keys.begin(), node->keys[index - 1]);
    if (child->is_leaf) {
        child->values.insert(child->values.begin(), node->values[index - 1]);
    }
    node->keys[index - 1] = sibling->keys.back();
    if (child->is_leaf) {
        node->values[index - 1] = sibling->values.back();
    }
    sibling->keys.pop_back();
    if (child->is_leaf) {
        sibling->values.pop_back();
    }

    if (!child->is_leaf) {
        child->children.insert(child->children.begin(), std::move(sibling->children.back()));
        sibling->children.pop_back();
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::borrow_from_next(BTreeNode* node, int index) {
    BTreeNode* child = node->children[index].get();
    BTreeNode* sibling = node->children[index + 1].get();

    child->keys.push_back(node->keys[index]);
    if (child->is_leaf) {
        child->values.push_back(node->values[index]);
    }
    node->keys[index] = sibling->keys.front();
    if (child->is_leaf) {
        node->values[index] = sibling->values.front();
    }

    sibling->keys.erase(sibling->keys.begin());
    if (child->is_leaf) {
        sibling->values.erase(sibling->values.begin());
    }

    if (!child->is_leaf) {
        child->children.push_back(std::move(sibling->children.front()));
        sibling->children.erase(sibling->children.begin());
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::merge(BTreeNode* node, int index) {
    BTreeNode* child = node->children[index].get();
    BTreeNode* sibling = node->children[index + 1].get();

    child->keys.push_back(node->keys[index]);
    // In a B-Tree, the median key from the parent moves down.
    // If we store values in internal nodes, we'd move the value too.
    // child->values.push_back(node->values[index]);

    for (size_t i = 0; i < sibling->keys.size(); ++i) {
        child->keys.push_back(sibling->keys[i]);
        if (child->is_leaf) {
             child->values.push_back(sibling->values[i]);
        }
    }

    if (!child->is_leaf) {
        for (size_t i = 0; i < sibling->children.size(); ++i) {
            child->children.push_back(std::move(sibling->children[i]));
        }
    }

    node->keys.erase(node->keys.begin() + index);
    if (child->is_leaf) {
        node->values.erase(node->values.begin() + index);
    }
    node->children.erase(node->children.begin() + index + 1);
}

// --- SERIALIZATION ---
template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::write_string(std::ostream& os, const std::string& s) {
    size_t len = s.length();
    os.write(reinterpret_cast<const char*>(&len), sizeof(len));
    os.write(s.c_str(), len);
}

template<typename Key, typename Value, int Order>
std::string BTree<Key, Value, Order>::read_string(std::istream& is) {
    size_t len;
    is.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::vector<char> buf(len);
    is.read(buf.data(), len);
    return std::string(buf.begin(), buf.end());
}


template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::dump(std::ostream& os) {
    if (!os) return;
    dump_node(os, root_.get());
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::load(std::istream& is) {
    if (!is) return;
    root_ = load_node(is);
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::dump_node(std::ostream& os, const BTreeNode* node) {
    if (!node) return;
    os.write(reinterpret_cast<const char*>(&node->is_leaf), sizeof(node->is_leaf));
    size_t key_count = node->keys.size();
    os.write(reinterpret_cast<const char*>(&key_count), sizeof(key_count));

    for (const auto& key : node->keys) {
        write_string(os, key);
    }
    if (node->is_leaf) {
        for (const auto& value : node->values) {
            write_string(os, value);
        }
    }

    if (!node->is_leaf) {
        for (const auto& child : node->children) {
            dump_node(os, child.get());
        }
    }
}

template<typename Key, typename Value, int Order>
std::unique_ptr<typename BTree<Key, Value, Order>::BTreeNode> BTree<Key, Value, Order>::load_node(std::istream& is) {
    bool is_leaf;
    is.read(reinterpret_cast<char*>(&is_leaf), sizeof(is_leaf));
    if (!is) return nullptr;

    auto node = std::make_unique<BTreeNode>(is_leaf);
    size_t key_count;
    is.read(reinterpret_cast<char*>(&key_count), sizeof(key_count));

    node->keys.resize(key_count);
    for (size_t i = 0; i < key_count; ++i) {
        node->keys[i] = read_string(is);
    }

    if (is_leaf) {
        node->values.resize(key_count);
        for (size_t i = 0; i < key_count; ++i) {
            node->values[i] = read_string(is);
        }
    }

    if (!is_leaf) {
        node->children.resize(key_count + 1);
        for (size_t i = 0; i < key_count + 1; ++i) {
            node->children[i] = load_node(is);
        }
    }
    return node;
}


// Explicit template instantiation
template class BTree<std::string, std::string>;

} // namespace Storage
} // namespace TissDB
