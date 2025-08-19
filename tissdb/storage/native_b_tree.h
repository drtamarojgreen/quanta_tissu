#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include <fstream>

namespace TissDB {
namespace Storage {

// A basic B-Tree implementation
template<typename Key, typename Value, int Order = 10>
class BTree {
public:
    BTree();
    ~BTree();

    void insert(const Key& key, const Value& value);
    std::optional<Value> find(const Key& key);
    void erase(const Key& key);

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

    // Helper methods
    void insert_non_full(BTreeNode* node, const Key& key, const Value& value);
    void split_child(BTreeNode* parent, int index);
    std::optional<Value> find_recursive(BTreeNode* node, const Key& key);
    // Erase methods
    void erase_recursive(BTreeNode* node, const Key& key);
    void remove_from_leaf(BTreeNode* node, int index);
    void remove_from_non_leaf(BTreeNode* node, int index);
    Key get_predecessor(BTreeNode* node, int index);
    Value get_predecessor_value(BTreeNode* node, int index);
    Key get_successor(BTreeNode* node, int index);
    Value get_successor_value(BTreeNode* node, int index);
    void fill(BTreeNode* node, int index);
    void borrow_from_prev(BTreeNode* node, int index);
    void borrow_from_next(BTreeNode* node, int index);
    void merge(BTreeNode* node, int index);
    int find_key(BTreeNode* node, const Key& key);

    // Dump/Load methods
    void dump_recursive(BTreeNode* node, std::ostream& os);
    std::unique_ptr<BTreeNode> load_recursive(std::istream& is);
};

} // namespace Storage
} // namespace TissDB
