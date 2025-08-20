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
    template<typename Func>
    void foreach_recursive(BTreeNode* node, Func& func);
public:
    template<typename Func>
    void foreach(Func func);
};

} // namespace Storage
} // namespace TissDB
