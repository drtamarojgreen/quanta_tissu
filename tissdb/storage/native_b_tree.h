#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include <fstream>
#include <algorithm> // For std::lower_bound

namespace TissDB {
namespace Storage {

template<typename Key, typename Value, int Order = 10>
class BTree {
public:
    BTree();
    ~BTree();

    void insert(const Key& key, const Value& value);
    std::optional<Value> find(const Key& key);
    void erase(const Key& key);
    std::vector<std::pair<Key, Value>> find_range(const Key& start_key, const Key& end_key);

    void dump(std::ostream& os);
    void load(std::istream& is);

private:
    struct BTreeNode {
        bool is_leaf;
        std::vector<Key> keys;
        std::vector<Value> values;
        std::vector<std::unique_ptr<BTreeNode>> children;

        BTreeNode(bool leaf = false);
        ~BTreeNode();
    };

    std::unique_ptr<BTreeNode> root_;

    // Insert helpers
    void insert_non_full(BTreeNode* node, const Key& key, const Value& value);
    void split_child(BTreeNode* parent, int index);

    // Find helper
    std::optional<Value> find_recursive(BTreeNode* node, const Key& key);
    void find_range_recursive(BTreeNode* node, const Key& start_key, const Key& end_key, std::vector<std::pair<Key, Value>>& result);

    // Erase helpers
    void erase_recursive(BTreeNode* node, const Key& key);
    void remove_from_leaf(BTreeNode* node, int index);
    void remove_from_non_leaf(BTreeNode* node, int index);
    Key get_predecessor(BTreeNode* node, int index);
    Key get_successor(BTreeNode* node, int index);
    void fill(BTreeNode* node, int index);
    void borrow_from_prev(BTreeNode* node, int index);
    void borrow_from_next(BTreeNode* node, int index);
    void merge(BTreeNode* node, int index);
    int find_key(BTreeNode* node, const Key& key);

    // Serialization helpers
    void dump_node(std::ostream& os, const BTreeNode* node);
    std::unique_ptr<BTreeNode> load_node(std::istream& is);
    void write_string(std::ostream& os, const std::string& s);
    std::string read_string(std::istream& is);

    // Iteration helper
public:
    template<typename Func>
    void foreach(Func func) {
        foreach_recursive(root_.get(), func);
    }
private:
    template<typename Func>
    void foreach_recursive(BTreeNode* node, Func& func) {
        if (node->is_leaf) {
            for (size_t i = 0; i < node->keys.size(); ++i) {
                func(node->keys[i], node->values[i]);
            }
        } else {
            for (size_t i = 0; i < node->children.size(); ++i) {
                foreach_recursive(node->children[i].get(), func);
            }
        }
    }
};

template<typename Key, typename Value, int Order>
std::vector<std::pair<Key, Value>> BTree<Key, Value, Order>::find_range(const Key& start_key, const Key& end_key) {
    std::vector<std::pair<Key, Value>> result;
    if (root_) {
        find_range_recursive(root_.get(), start_key, end_key, result);
    }
    return result;
}

template<typename Key, typename Value, int Order>
void BTree<Key, Value, Order>::find_range_recursive(BTreeNode* node, const Key& start_key, const Key& end_key, std::vector<std::pair<Key, Value>>& result) {
    if (!node) {
        return;
    }

    size_t i = 0;
    // Find the first key in the node that is >= start_key
    while (i < node->keys.size() && node->keys[i] < start_key) {
        i++;
    }

    // Recursively visit the left subtree if it's not a leaf
    if (!node->is_leaf) {
        find_range_recursive(node->children[i].get(), start_key, end_key, result);
    }

    // Process keys in the current node and the children to their right
    while (i < node->keys.size() && node->keys[i] <= end_key) {
        result.push_back({node->keys[i], node->values[i]});
        if (!node->is_leaf) {
            find_range_recursive(node->children[i + 1].get(), start_key, end_key, result);
        }
        i++;
    }
}

} // namespace Storage
} // namespace TissDB
