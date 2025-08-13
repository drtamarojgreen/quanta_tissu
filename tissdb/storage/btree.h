
#pragma once

#include <string>
#include <vector>
#include <memory>

namespace TissDB {
namespace Storage {

// Represents a node in the B-Tree
struct BTreeNode {
    bool is_leaf;
    std::vector<std::string> keys;
    std::vector<std::string> doc_ids; // Only used in leaf nodes
    std::vector<std::unique_ptr<BTreeNode>> children; // Only used in internal nodes
};

// Represents a B-Tree index
class BTree {
public:
    BTree();

    void insert(const std::string& key, const std::string& doc_id);
    std::vector<std::string> find(const std::string& key);
    void remove(const std::string& key);

    void serialize(std::ostream& os);
    void deserialize(std::istream& is);

private:
    std::unique_ptr<BTreeNode> root;
    int t; // Minimum degree

    // Helper functions
    void insert_non_full(BTreeNode* node, const std::string& key, const std::string& doc_id);
    void split_child(BTreeNode* parent, int i);
    BTreeNode* search(BTreeNode* node, const std::string& key);
};

}
}
