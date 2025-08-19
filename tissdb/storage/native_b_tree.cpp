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
    if (!root_) return;
    erase_recursive(root_.get(), key);
    if (root_->keys.empty() && !root_->is_leaf) {
        root_ = std::move(root_->children[0]);
    }
}

template<typename Key, typename Value, int Order>
int BTree<Key, Value, Order>::find_key(BTreeNode* node, const Key& key) {
    int idx = 0;
    while (idx < node->keys.size() && node->keys[idx] < key)
        ++idx;
    return idx;
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
        Value pred_val = get_predecessor_value(node, index);
        node->keys[index] = pred_key;
        node->values[index] = pred_val;
        erase_recursive(node->children[index].get(), pred_key);
    } else if (node->children[index + 1]->keys.size() >= Order) {
        Key succ_key = get_successor(node, index);
        Value succ_val = get_successor_value(node, index);
        node->keys[index] = succ_key;
        node->values[index] = succ_val;
        erase_recursive(node->children[index + 1].get(), succ_key);
    } else {
        merge(node, index);
        erase_recursive(node->children[index].get(), key);
    }
}

template<typename Key, typename Value, int Order>
Key BTree<Key, Value, Order>::get_predecessor(BTreeNode* node, int index) {
    BTreeNode* cur = node->children[index].get();
    while (!cur->is_leaf)
        cur = cur->children[cur->keys.size()].get();
    return cur->keys.back();
}

template<typename Key, typename Value, int Order>
Value BTree<Key, Value, Order>::get_predecessor_value(BTreeNode* node, int index) {
    BTreeNode* cur = node->children[index].get();
    while (!cur->is_leaf)
        cur = cur->children[cur->keys.size()].get();
    return cur->values.back();
}

template<typename Key, typename Value, int Order>
Key BTree<Key, Value, Order>::get_successor(BTreeNode* node, int index) {
    BTreeNode* cur = node->children[index + 1].get();
    while (!cur->is_leaf)
        cur = cur->children[0].get();
    return cur->keys.front();
}

template<typename Key, typename Value, int Order>
Value BTree<Key, Value, Order>::get_successor_value(BTreeNode* node, int index) {
    BTreeNode* cur = node->children[index + 1].get();
    while (!cur->is_leaf)
        cur = cur->children[0].get();
    return cur->values.front();
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::fill(BTreeNode* node, int index) {
    if (index != 0 && node->children[index - 1]->keys.size() >= Order)
        borrow_from_prev(node, index);
    else if (index != node->keys.size() && node->children[index + 1]->keys.size() >= Order)
        borrow_from_next(node, index);
    else {
        if (index != node->keys.size())
            merge(node, index);
        else
            merge(node, index - 1);
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::borrow_from_prev(BTreeNode* node, int index) {
    BTreeNode* child = node->children[index].get();
    BTreeNode* sibling = node->children[index - 1].get();

    child->keys.insert(child->keys.begin(), node->keys[index - 1]);
    child->values.insert(child->values.begin(), node->values[index - 1]);

    node->keys[index - 1] = sibling->keys.back();
    node->values[index - 1] = sibling->values.back();

    if (!child->is_leaf) {
        child->children.insert(child->children.begin(), std::move(sibling->children.back()));
        sibling->children.pop_back();
    }

    sibling->keys.pop_back();
    sibling->values.pop_back();
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::borrow_from_next(BTreeNode* node, int index) {
    BTreeNode* child = node->children[index].get();
    BTreeNode* sibling = node->children[index + 1].get();

    child->keys.push_back(node->keys[index]);
    child->values.push_back(node->values[index]);

    node->keys[index] = sibling->keys.front();
    node->values[index] = sibling->values.front();

    if (!child->is_leaf) {
        child->children.push_back(std::move(sibling->children.front()));
        sibling->children.erase(sibling->children.begin());
    }

    sibling->keys.erase(sibling->keys.begin());
    sibling->values.erase(sibling->values.begin());
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::merge(BTreeNode* node, int index) {
    BTreeNode* child = node->children[index].get();
    BTreeNode* sibling = node->children[index + 1].get();

    child->keys.push_back(node->keys[index]);
    child->values.push_back(node->values[index]);

    child->keys.insert(child->keys.end(), sibling->keys.begin(), sibling->keys.end());
    child->values.insert(child->values.end(), sibling->values.begin(), sibling->values.end());

    if (!child->is_leaf) {
        child->children.insert(child->children.end(),
                               std::make_move_iterator(sibling->children.begin()),
                               std::make_move_iterator(sibling->children.end()));
    }

    node->keys.erase(node->keys.begin() + index);
    node->values.erase(node->values.begin() + index);
    node->children.erase(node->children.begin() + index + 1);
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::erase_recursive(BTreeNode* node, const Key& key) {
    int idx = find_key(node, key);

    if (idx < node->keys.size() && node->keys[idx] == key) {
        if (node->is_leaf)
            remove_from_leaf(node, idx);
        else
            remove_from_non_leaf(node, idx);
    } else {
        if (node->is_leaf) return;

        bool flag = (idx == node->keys.size());

        if (node->children[idx]->keys.size() < Order)
            fill(node, idx);

        if (flag && idx > node->keys.size())
            erase_recursive(node->children[idx - 1].get(), key);
        else
            erase_recursive(node->children[idx].get(), key);
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::dump(std::ostream& os) {
    if (root_) {
        dump_recursive(root_.get(), os);
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::dump_recursive(BTreeNode* node, std::ostream& os) {
    os.write(reinterpret_cast<const char*>(&node->is_leaf), sizeof(node->is_leaf));
    size_t num_keys = node->keys.size();
    os.write(reinterpret_cast<const char*>(&num_keys), sizeof(num_keys));
    for (const auto& key : node->keys) {
        size_t key_len = key.size();
        os.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        os.write(key.data(), key_len);
    }
    for (const auto& value : node->values) {
        size_t val_len = value.size();
        os.write(reinterpret_cast<const char*>(&val_len), sizeof(val_len));
        os.write(value.data(), val_len);
    }

    if (!node->is_leaf) {
        for (const auto& child : node->children) {
            dump_recursive(child.get(), os);
        }
    }
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::load(std::istream& is) {
    if (is.peek() != EOF) {
        root_ = load_recursive(is);
    }
}

template<typename Key, typename Value, int Order>
std::unique_ptr<typename BTree<Key, Value, Order>::BTreeNode> BTree<Key, Value, Order>::load_recursive(std::istream& is) {
    auto node = std::make_unique<BTreeNode>();
    is.read(reinterpret_cast<char*>(&node->is_leaf), sizeof(node->is_leaf));
    size_t num_keys;
    is.read(reinterpret_cast<char*>(&num_keys), sizeof(num_keys));
    node->keys.resize(num_keys);
    node->values.resize(num_keys);
    for (size_t i = 0; i < num_keys; ++i) {
        size_t key_len;
        is.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        node->keys[i].resize(key_len);
        is.read(&node->keys[i][0], key_len);
    }
    for (size_t i = 0; i < num_keys; ++i) {
        size_t val_len;
        is.read(reinterpret_cast<char*>(&val_len), sizeof(val_len));
        node->values[i].resize(val_len);
        is.read(&node->values[i][0], val_len);
    }

    if (!node->is_leaf) {
        node->children.resize(num_keys + 1);
        for (size_t i = 0; i < num_keys + 1; ++i) {
            node->children[i] = load_recursive(is);
        }
    }
    return node;
}

// Explicit template instantiation
template class BTree<std::string, std::string>;

} // namespace Storage
} // namespace TissDB
