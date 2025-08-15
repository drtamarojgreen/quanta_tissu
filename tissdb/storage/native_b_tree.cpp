#include "native_b_tree.h"

namespace TissDB {
namespace Storage {

template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::BTreeNode::BTreeNode(bool leaf) : is_leaf(leaf) {}

template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::BTreeNode::~BTreeNode() {}

template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::BTree() {
    root_ = std::make_unique<BTreeNode>(true);
}

template<typename Key, typename Value, int Order>
BTree<Key, Value, Order>::~BTree() {}

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
    int i = node->keys.size() - 1;
    if (node->is_leaf) {
        node->keys.push_back(key);
        node->values.push_back(value);
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->values[i + 1] = value;
    } else {
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
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
    parent->values.insert(parent->values.begin() + index, child->values[Order - 1]);
    parent->children.insert(parent->children.begin() + index + 1, std::move(new_child));

    new_child->keys.assign(child->keys.begin() + Order, child->keys.end());
    new_child->values.assign(child->values.begin() + Order, child->values.end());
    child->keys.resize(Order - 1);
    child->values.resize(Order - 1);

    if (!child->is_leaf) {
        new_child->children.assign(
            std::make_move_iterator(child->children.begin() + Order),
            std::make_move_iterator(child->children.end())
        );
        child->children.resize(Order);
    }
}

template<typename Key, typename Value, int Order>
std::optional<Value> BTree<Key, Value, Order>::find(const Key& key) {
    return find_recursive(root_.get(), key);
}

template<typename Key, typename Value, int Order>
std::optional<Value> BTree<Key, Value, Order>::find_recursive(BTreeNode* node, const Key& key) {
    int i = 0;
    while (i < node->keys.size() && key > node->keys[i]) {
        i++;
    }

    if (i < node->keys.size() && key == node->keys[i]) {
        return node->values[i];
    } else if (node->is_leaf) {
        return std::nullopt;
    } else {
        return find_recursive(node->children[i].get(), key);
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::erase(const Key& key) {
    // Placeholder for erase
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::dump(std::ostream& os) {
    // Placeholder for dump
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::load(std::istream& is) {
    // Placeholder for load
}

// Explicit template instantiation
template class BTree<std::string, std::string>;

} // namespace Storage
} // namespace TissDB
