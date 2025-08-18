//
// Created by 刘育氚 on 2019/5/24.
//

#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include <algorithm>
#include <string>
#include <cstring>
#include <vector>
#include "Comp.h"
#include "Utils.h"
#include <cassert>

const unsigned int MIN_ORDER = 2;

template<typename K, typename V, class Comp = DefaultCompare<K>>
class BPTree {
private:
    static const std::string SUFFIX;

    struct Node {
        Node *parentPtr = NULL;
        Node *previous = NULL;
        Node *next = NULL;
        bool leaf;
        std::vector<Node *> childNodePtrs;
        std::vector<V> values;
        std::vector<K> keys;

        Node(unsigned int initCap, bool leaf, Comp comp);
    };

    unsigned int order;
    unsigned int size;
    unsigned int minLoad;
    Comp comp;
    unsigned int initCap;
    Node *root;

    bool putToNode(Node *nodePtr, const K &key, const V *value, Node *insertNodePtr);

    int deleteFromNode(Node *nodePtr, const K &key);

    void updateParentKey(Node *node, K &oldKey, K &newKey, int pos);

    Node *fixNode(Node *nodePtr);

    Node *split(Node *nodePtr);

    Node *getFirstLeaf();

    Node *getLastLeaf();

    void clear(Node *node);

    // return node's offset
    long serializeNode(const Node *node, FILE *f);

    // ftell must be at the start offset of target node
    Node *deserializeNode(FILE *f, Node *parentNode);

    /**
     * @return NULL if this node is last node of its layer
     */
    Node *getNextSibling(Node *node);

    void recoverLinkList(bool valid);

    Node *copyNode(Node *parentPtr, Node *fromPtr);

public:

    explicit BPTree(unsigned int order) :
            order(std::max(MIN_ORDER, order)),
            size(0),
            minLoad((this->order + 1) / 2),
            comp(),
            initCap(std::max(std::min(this->order,
                                      initCap), this->minLoad)),
            root(new Node(this->initCap,
                          true, comp)) {}

    BPTree(unsigned int order, Comp comp) : BPTree(order, 2, comp) {}

    explicit BPTree(unsigned int order, unsigned int initCap, Comp comp) :
            order(std::max(MIN_ORDER, order)),
            size(0),
            minLoad((this->order + 1) / 2),
            comp(comp),
            initCap(std::max(std::min(this->order,
                                      initCap), this->minLoad)),
            root(new Node(this->initCap,
                          true, comp)) {}

    BPTree(const BPTree &another) :
            order(another.order),
            size(another.size),
            minLoad(another.minLoad),
            comp(another.comp),
            initCap(another.initCap) {
        root = copyNode(NULL, another.root);
        recoverLinkList(false);
    }

    ~BPTree();

    /**
     * deserialize from a file
     */
    static std::shared_ptr<BPTree<K, V, Comp>> deserialize(const std::string &path, Comp comp);

    void put(const K &key, const V &value);

    void remove(K &key);

    /**
     * @return NULL if not exists else a pointer to the value
     */
    V *get(const K &key);

    bool containsKey(const K &key);

    unsigned int getOrder();

    unsigned int getSize();

    /**
     * iterate order by key
     * @param func call func(const K& key, const V& value) for each. func returns true means iteration ends
     */
    template<typename BiApply>
    void foreach(BiApply func);

    template<typename BiApply>
    void foreachReverse(BiApply func);

    /**
     * iterate order by key with index (starts from 0)
     * @param func call func(int index, const K& key, const V& value) for each. func returns true means iteration ends
     */
    template<typename BiApplyIndex>
    void foreachIndex(BiApplyIndex func);

    template<typename BiApplyIndex>
    void foreachIndexReverse(BiApplyIndex func);

    void serialize(std::string &path);

    /**
     * clear the tree
     * note that all values allocated will be freed
     */
    void clear();
};

template<typename K, typename V, class Comp>
BPTree<K, V, Comp>::Node::Node(unsigned int initCap, bool leaf, Comp comp) :
        leaf(leaf) {
    childNodePtrs.reserve(initCap);
    if (leaf) {
        values.reserve(initCap);
    }
    keys.reserve(initCap);
}


template<typename K, typename V, class Comp>
const std::string BPTree<K, V, Comp>::SUFFIX = ".bpt";

template<typename K, typename V, class Comp>
void BPTree<K, V, Comp>::put(const K &key, const V &value) {
    bool result;
    Node *insertNode = root;
    int pos = -1;
    while (!insertNode->leaf) {
        auto it = std::lower_bound(insertNode->keys.begin(), insertNode->keys.end(), key, comp);
        pos = std::distance(insertNode->keys.begin(), it);
        if (pos == insertNode->childNodePtrs.size()) {
            pos--;
        }
        insertNode = insertNode->childNodePtrs[pos];
    }

    result = putToNode(insertNode, key, &value, NULL);
    if (result) size++;

    Node *tmp = insertNode->parentPtr;
    while (tmp) {
        auto &keys = tmp->keys;
        K oldKey = keys[pos];
        if (pos >= 0 && pos < keys.size() && comp(oldKey, key) < 0) {
            keys[pos] = key;
            if (pos == keys.size() - 1 && (tmp = tmp->parentPtr)) {
                auto it = std::lower_bound(tmp->keys.begin(), tmp->keys.end(), oldKey, comp);
                pos = std::distance(tmp->keys.begin(), it);
                continue;
            }
        }
        break;
    }

    if (result && insertNode->keys.size() > order) {
        Node *node = insertNode;
        Node *splitParent;
        Node *p;
        do {
            p = node->parentPtr;
            splitParent = split(node);
            if (splitParent != p) {
                this->root = splitParent;
                break;
            } else if (splitParent->keys.size() > order) {
                node = splitParent;
            } else {
                break;
            }
        } while (true);
    }
}

template<typename K, typename V, class Comp>
void BPTree<K, V, Comp>::remove(K &key) {
    Node *deleteNodePtr = root;
    int pos = -1;
    while (!deleteNodePtr->leaf) {
        auto it = std::lower_bound(deleteNodePtr->keys.begin(), deleteNodePtr->keys.end(), key, comp);
        pos = std::distance(deleteNodePtr->keys.begin(), it);
        if (pos == deleteNodePtr->childNodePtrs.size()) {
            pos--;
        }
        deleteNodePtr = deleteNodePtr->childNodePtrs[pos];
    }

    int deleteKeyPos = deleteFromNode(deleteNodePtr, key);
    if (deleteKeyPos < 0 || deleteNodePtr == root) {
        return;
    }
    if (pos == -1) {
        pos = deleteKeyPos;
    }

    if (deleteKeyPos == deleteNodePtr->keys.size()) {
        updateParentKey(deleteNodePtr, key, deleteNodePtr->keys.back(), pos);
    }

    Node *tmpNodePtr = deleteNodePtr;
    while ((tmpNodePtr = fixNode(tmpNodePtr)));
    if (root->childNodePtrs.size() == 1) {
        Node *oldRoot = root;
        root = root->childNodePtrs[0];
        root->parentPtr = NULL;
        delete oldRoot;
    }
}

template<typename K, typename V, class Comp>
V *BPTree<K, V, Comp>::get(const K &key) {
    Node *curPtr = root;
    int pos;
    while (!curPtr->leaf) {
        auto it = std::lower_bound(curPtr->keys.begin(), curPtr->keys.end(), key, comp);
        pos = std::distance(curPtr->keys.begin(), it);
        if (pos == curPtr->childNodePtrs.size()) {
            pos--;
        }
        curPtr = curPtr->childNodePtrs[pos];
    }
    auto it = std::lower_bound(curPtr->keys.begin(), curPtr->keys.end(), key, comp);
    pos = std::distance(curPtr->keys.begin(), it);
    if (pos < curPtr->values.size() && comp(curPtr->keys[pos], key) == 0) {
        return &(curPtr->values[pos]);
    }

    return NULL;
}

template<typename K, typename V, class Comp>
unsigned int BPTree<K, V, Comp>::getOrder() {
    return order;
}

template<typename K, typename V, class Comp>
unsigned int BPTree<K, V, Comp>::getSize() {
    return size;
}

template<typename K, typename V, class Comp>
bool BPTree<K, V, Comp>::putToNode(BPTree::Node *nodePtr, const K &key, const V *value, BPTree::Node *insertNodePtr) {
    auto it = std::lower_bound(nodePtr->keys.begin(), nodePtr->keys.end(), key, comp);
    int toIndex = std::distance(nodePtr->keys.begin(), it);
    bool present = toIndex < nodePtr->keys.size() && comp(key, nodePtr->keys[toIndex]) == 0;

    if (present) {
        if (nodePtr->leaf) {
            nodePtr->values[toIndex] = *value;
        } else {
            nodePtr->childNodePtrs[toIndex] = insertNodePtr;
        }
        return false;
    } else {
        nodePtr->keys.insert(it, key);
        if (nodePtr->leaf) {
            nodePtr->values.insert(nodePtr->values.begin() + toIndex, *value);
        } else {
            nodePtr->childNodePtrs.insert(nodePtr->childNodePtrs.begin() + toIndex, insertNodePtr);
        }
    }

    return true;
}

template<typename K, typename V, class Comp>
typename BPTree<K, V, Comp>::Node *BPTree<K, V, Comp>::split(BPTree::Node *nodePtr) {
    if (nodePtr->keys.size() <= order) {
        return NULL;
    }

    int mid = (nodePtr->keys.size() + 1) / 2;
    Node *parentPtr = nodePtr->parentPtr;
    if (!parentPtr) {
        parentPtr = new Node(initCap, false, comp);
        nodePtr->parentPtr = parentPtr;
    }

    // split
    Node *left = new Node(initCap, nodePtr->leaf, comp);
    left->keys.insert(left->keys.end(), nodePtr->keys.begin(), nodePtr->keys.begin() + mid);
    nodePtr->keys.erase(nodePtr->keys.begin(), nodePtr->keys.begin() + mid);
    left->parentPtr = parentPtr;

    if (nodePtr->leaf) {
        left->values.insert(left->values.end(), nodePtr->values.begin(), nodePtr->values.begin() + mid);
        nodePtr->values.erase(nodePtr->values.begin(), nodePtr->values.begin() + mid);
        left->previous = nodePtr->previous;
        nodePtr->previous = left;
        left->next = nodePtr;
        if (left->previous) {
            left->previous->next = left;
        }
    } else {
        left->childNodePtrs.insert(left->childNodePtrs.end(), nodePtr->childNodePtrs.begin(), nodePtr->childNodePtrs.begin() + mid);
        nodePtr->childNodePtrs.erase(nodePtr->childNodePtrs.begin(), nodePtr->childNodePtrs.begin() + mid);
        for (Node *&n : left->childNodePtrs) {
            n->parentPtr = left;
        }
    }
    putToNode(parentPtr, left->keys.back(), NULL, left);
    putToNode(parentPtr, nodePtr->keys.back(), NULL, nodePtr);
    return parentPtr;
}

template<typename K, typename V, class Comp>
int BPTree<K, V, Comp>::deleteFromNode(BPTree::Node *nodePtr, const K &key) {
    auto it = std::lower_bound(nodePtr->keys.begin(), nodePtr->keys.end(), key, comp);
    int pos = std::distance(nodePtr->keys.begin(), it);
    if (pos >= nodePtr->keys.size() || comp(nodePtr->keys[pos], key) != 0) {
        return -1;
    }
    nodePtr->keys.erase(it);
    if (nodePtr->leaf) {
        nodePtr->values.erase(nodePtr->values.begin() + pos);
    } else {
        Node *node = nodePtr->childNodePtrs[pos];
        nodePtr->childNodePtrs.erase(nodePtr->childNodePtrs.begin() + pos);
        delete node;
    }
    return pos;
}

template<typename K, typename V, class Comp>
typename BPTree<K, V, Comp>::Node *BPTree<K, V, Comp>::fixNode(BPTree::Node *nodePtr) {
    if (minLoad <= nodePtr->keys.size() || !nodePtr->parentPtr) {
        return NULL;
    }

    auto &parentKeys = nodePtr->parentPtr->keys;
    K &key = nodePtr->keys.back();
    auto it = std::lower_bound(nodePtr->parentPtr->keys.begin(), nodePtr->parentPtr->keys.end(), key, comp);
    int pos = std::distance(nodePtr->parentPtr->keys.begin(), it);

    Node *sibling;
    // borrow from previous
    int previousIndex = pos - 1;
    if (previousIndex >= 0) {
        sibling = nodePtr->parentPtr->childNodePtrs[previousIndex];
        if (sibling->keys.size() > minLoad) {
            K &oldKey = sibling->keys.back();
            K &newKey = sibling->keys[sibling->keys.size() - 2];
            nodePtr->keys.insert(nodePtr->keys.begin(), sibling->keys.back());
            sibling->keys.pop_back();

            if (nodePtr->leaf) {
                nodePtr->values.insert(nodePtr->values.begin(), sibling->values.back());
                sibling->values.pop_back();
            } else {
                nodePtr->childNodePtrs.insert(nodePtr->childNodePtrs.begin(), sibling->childNodePtrs.back());
                sibling->childNodePtrs.back()->parentPtr = nodePtr;
                sibling->childNodePtrs.pop_back();
            }
            updateParentKey(sibling, oldKey, newKey, previousIndex);
            return NULL;
        }
    }

    // borrow from next
    int nextIndex = pos + 1;
    if (nextIndex < parentKeys.size()) {
        sibling = nodePtr->parentPtr->childNodePtrs[nextIndex];
        if (sibling->keys.size() > minLoad) {
            K &oldKey = nodePtr->keys.back();
            K &newKey = sibling->keys.front();
            nodePtr->keys.push_back(newKey);
            sibling->keys.erase(sibling->keys.begin());

            if (nodePtr->leaf) {
                nodePtr->values.push_back(sibling->values.front());
                sibling->values.erase(sibling->values.begin());
            } else {
                nodePtr->childNodePtrs.push_back(sibling->childNodePtrs.front());
                sibling->childNodePtrs.front()->parentPtr = nodePtr;
                sibling->childNodePtrs.erase(sibling->childNodePtrs.begin());
            }
            updateParentKey(nodePtr, oldKey, newKey, pos);
            return NULL;
        }
    }

    // merge with previous
    if (previousIndex >= 0) {
        sibling = nodePtr->parentPtr->childNodePtrs[previousIndex];
        nodePtr->keys.insert(nodePtr->keys.begin(), sibling->keys.begin(), sibling->keys.end());
        if (nodePtr->leaf) {
            nodePtr->values.insert(nodePtr->values.begin(), sibling->values.begin(), sibling->values.end());
            if (sibling->previous) {
                sibling->previous->next = nodePtr;
            }
            nodePtr->previous = sibling->previous;
        } else {
            nodePtr->childNodePtrs.insert(nodePtr->childNodePtrs.begin(), sibling->childNodePtrs.begin(), sibling->childNodePtrs.end());
            for (Node *&n : sibling->childNodePtrs) {
                n->parentPtr = nodePtr;
            }
        }
        nodePtr->parentPtr->keys.erase(nodePtr->parentPtr->keys.begin() + previousIndex);
        nodePtr->parentPtr->childNodePtrs.erase(nodePtr->parentPtr->childNodePtrs.begin() + previousIndex);
        delete sibling;
        return nodePtr->parentPtr;
    }

    if (nextIndex < parentKeys.size()) {
        sibling = nodePtr->parentPtr->childNodePtrs[nextIndex];
        sibling->keys.insert(sibling->keys.begin(), nodePtr->keys.begin(), nodePtr->keys.end());
        if (nodePtr->leaf) {
            sibling->values.insert(sibling->values.begin(), nodePtr->values.begin(), nodePtr->values.end());
            if (nodePtr->previous) {
                nodePtr->previous->next = sibling;
            }
            sibling->previous = nodePtr->previous;
        } else {
            sibling->childNodePtrs.insert(sibling->childNodePtrs.begin(), nodePtr->childNodePtrs.begin(), nodePtr->childNodePtrs.end());
            for (Node *&n : nodePtr->childNodePtrs) {
                n->parentPtr = sibling;
            }
        }
        nodePtr->parentPtr->keys.erase(nodePtr->parentPtr->keys.begin() + pos);
        nodePtr->parentPtr->childNodePtrs.erase(nodePtr->parentPtr->childNodePtrs.begin() + pos);
        delete nodePtr;
        return sibling->parentPtr;
    }

    // parent is root
    if (nodePtr->parentPtr == root) {
        root->childNodePtrs.clear();
        root->values.insert(root->values.end(), nodePtr->values.begin(), nodePtr->values.end());
        root->keys.insert(root->keys.end(), nodePtr->keys.begin(), nodePtr->keys.end());
        return NULL;
    }

    return NULL;
}


template<typename K, typename V, class Comp>
void BPTree<K, V, Comp>::updateParentKey(BPTree::Node *node, K &oldKey, K &newKey, int pos) {
    Node *tmpNodePtr = node;
    // delete old key, update parent
    while (tmpNodePtr && tmpNodePtr->parentPtr) {
        tmpNodePtr = tmpNodePtr->parentPtr;
        tmpNodePtr->keys[pos] = newKey;
        if (pos == tmpNodePtr->keys.size() - 1 && tmpNodePtr->parentPtr) {
            auto it = std::lower_bound(tmpNodePtr->parentPtr->keys.begin(), tmpNodePtr->parentPtr->keys.end(), oldKey, comp);
            pos = std::distance(tmpNodePtr->parentPtr->keys.begin(), it);
            continue;
        }
        break;
    }
}

template<typename K, typename V, class Comp>
void BPTree<K, V, Comp>::clear(BPTree::Node *node) {
    if (!node->leaf) {
        for (Node *child : node->childNodePtrs) {
            clear(child);
        }
    }

    for (Node *child : node->childNodePtrs) {
        delete child;
    }
    node->childNodePtrs.clear();
    node->keys.clear();
    node->values.clear();
}

template<typename K, typename V, class Comp>
void BPTree<K, V, Comp>::clear() {
    if (size == 0)
        return;
    clear(root);
    root->leaf = true;
    size = 0;
}

template<typename K, typename V, class Comp>
bool BPTree<K, V, Comp>::containsKey(const K &key) {
    Node *curPtr = root;
    int pos;
    while (!curPtr->leaf) {
        auto it = std::lower_bound(curPtr->keys.begin(), curPtr->keys.end(), key, comp);
        pos = std::distance(curPtr->keys.begin(), it);
        if (pos == curPtr->childNodePtrs.size()) {
            pos--;
        }
        curPtr = curPtr->childNodePtrs[pos];
    }
    auto it = std::lower_bound(curPtr->keys.begin(), curPtr->keys.end(), key, comp);
    pos = std::distance(curPtr->keys.begin(), it);
    if (pos < curPtr->values.size()) {
        return comp(curPtr->keys[pos], key) == 0;
    }

    return false;
}

template<typename K, typename V, class Comp>
template<class BiApply>
void BPTree<K, V, Comp>::foreach(BiApply func) {
    Node *node = getFirstLeaf();
    while (node) {
        for (size_t i = 0; i < node->values.size(); ++i) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(constKey, constValue)) {
                return;
            }
        }
        node = node->next;
    }
}

template<typename K, typename V, class Comp>
template<class BiApplyIndex>
void BPTree<K, V, Comp>::foreachIndex(BiApplyIndex func) {
    Node *node = getFirstLeaf();
    int index = 0;
    while (node) {
        for (size_t i = 0; i < node->values.size(); ++i, index++) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(index, constKey, constValue)) {
                return;
            }
        }
        node = node->next;
    }
}

template<typename K, typename V, class Comp>
typename BPTree<K, V, Comp>::Node *BPTree<K, V, Comp>::getFirstLeaf() {
    Node *node = root;
    while (!node->leaf) {
        node = node->childNodePtrs[0];
    }
    return node;
}


template<typename K, typename V, class Comp>
typename BPTree<K, V, Comp>::Node *BPTree<K, V, Comp>::getLastLeaf() {
    Node *node = root;
    while (!node->leaf) {
        node = node->childNodePtrs.back();
    }
    return node;
}

template<typename K, typename V, class Comp>
template<class BiApply>
void BPTree<K, V, Comp>::foreachReverse(BiApply func) {
    Node *node = getLastLeaf();
    while (node) {
        for (int i = node->values.size() - 1; i >= 0; --i) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(constKey, constValue)) {
                return;
            }
        }
        node = node->previous;
    }
}

template<typename K, typename V, class Comp>
template<class BiApplyIndex>
void BPTree<K, V, Comp>::foreachIndexReverse(BiApplyIndex func) {
    Node *node = getLastLeaf();
    int index = size - 1;
    while (node) {
        for (int i = node->values.size() - 1; i >= 0; --i, --index) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(index, constKey, constValue)) {
                return;
            }
        }
        node = node->previous;
    }
}

template<typename K, typename V, class Comp>
void BPTree<K, V, Comp>::serialize(std::string &path) {
    if (!path.rfind(SUFFIX)) {
        path = path += SUFFIX;
    }
    char *cPath = const_cast<char *>(path.c_str());
    FILE *f = bp_tree_utils::fopen(cPath, "w");

    // head: tree info
    bp_tree_utils::fwrite("LYCBP", 1, 5, f);
    bp_tree_utils::writeValLittle((unsigned char) sizeof(unsigned short), f);
    bp_tree_utils::writeValLittle((unsigned char) sizeof(unsigned int), f);
    bp_tree_utils::writeValLittle((unsigned char) sizeof(unsigned long), f);
    bp_tree_utils::writeValLittle((unsigned int) sizeof(K), f);
    bp_tree_utils::writeValLittle((unsigned int) sizeof(V), f);
    bp_tree_utils::writeValLittle(order, f);
    bp_tree_utils::writeValLittle(initCap, f);
    bp_tree_utils::writeValLittle(size, f);
    if (size > 0) {
        const BPTree::Node *constRoot = root;
        serializeNode(constRoot, f);
    }
    fflush(f);
    fclose(f);
}

template<typename K, typename V, class Comp>
long BPTree<K, V, Comp>::serializeNode(const BPTree::Node *node, FILE *f) {
    auto myOffset = ftell(f);

    bp_tree_utils::writeVal(node->leaf, f);
    unsigned int s = node->keys.size();
    assert(s >= minLoad || !node->parentPtr);
    bp_tree_utils::writeValLittle(s, f);
    for (unsigned int i = 0; i < s; ++i) {
        bp_tree_utils::writeValLittle(node->keys[i], f);
    }

    // leaf: write
    if (node->leaf) {
        for (unsigned int i = 0; i < s; ++i) {
            bp_tree_utils::writeValLittle(node->values[i], f);
        }
    } else {
        const auto &children = node->childNodePtrs;
        long childPtrsStartOffset = ftell(f);
        std::unique_ptr<long[]> childOffsets(new long[s]);
        bp_tree_utils::writeArrayLittle(childOffsets.get(), s, f);
        for (unsigned int i = 0; i < s; ++i) {
            const Node *nPtr = children[i];
            childOffsets[i] = serializeNode(nPtr, f);
        }
        long childEndOffset = ftell(f);
        fseek(f, childPtrsStartOffset, SEEK_SET);
        bp_tree_utils::writeArrayLittle(childOffsets.get(), s, f);
        fseek(f, childEndOffset, SEEK_SET);
    }

    return myOffset;
}

template<typename K, typename V, class Comp>
std::shared_ptr<BPTree<K, V, Comp>> BPTree<K, V, Comp>::deserialize(const std::string &path, Comp comp) {
    FILE *f = bp_tree_utils::fopen(path.c_str(), "r");
    char buf[6];
    bp_tree_utils::fread(buf, 1, 5, f);
    buf[5] = 0;
    if (strcmp(buf, "LYCBP") != 0) {
        throw bp_tree_utils::stringFormat("Check file header failed: expected 'LYCBP' but got '%s' (offset: 0)", buf);
    }

    bp_tree_utils::fread(buf, 1, 3, f);
    if (buf[0] != sizeof(short) || buf[1] != sizeof(unsigned int) || buf[2] != sizeof(long)) {
        throw std::string("this file is not compatible with machine");
    }
    auto sizeofK = bp_tree_utils::readValLittle<unsigned int>(f);
    if (sizeofK < sizeof(K)) {
        throw bp_tree_utils::stringFormat("Wrong sizeof(K): expected %d but got %d (offset: 8)", sizeof(K), sizeofK);
    }

    auto sizeofV = bp_tree_utils::readValLittle<unsigned int>(f);
    if (sizeofV != sizeof(V)) {
        throw bp_tree_utils::stringFormat("Wrong sizeof(V): expected %d but got %d (offset: 12)", sizeof(V), sizeofV);
    }


    auto order = bp_tree_utils::readValLittle<unsigned int>(f);
    if (order < 2) {
        throw bp_tree_utils::stringFormat("Wrong order: %d (offset: 16)", order);
    }

    auto initCap = bp_tree_utils::readValLittle<unsigned int>(f);
    if (initCap > order) {
        throw bp_tree_utils::stringFormat("Wrong initCap: %d (offset: 20)", initCap);
    }

    auto size = bp_tree_utils::readValLittle<unsigned int>(f);
    if (size < 0) {
        throw bp_tree_utils::stringFormat("Wrong size: %d (offset: 24)", initCap);
    }
    std::shared_ptr<BPTree> treePtr(new BPTree(order, initCap, comp));
    treePtr->size = size;

    if (size > 0) {
        treePtr->root = treePtr->deserializeNode(f, NULL);
        Node *pre = NULL;
        Node *node = treePtr->getFirstLeaf();
        while (node) {
            node->previous = pre;
            if (pre) {
                pre->next = node;
            }
            pre = node;
            node = treePtr->getNextSibling(node);
        }
    }

    treePtr->recoverLinkList(true);

    fclose(f);
    return treePtr;
}

template<typename K, typename V, class Comp>
typename BPTree<K, V, Comp>::Node *BPTree<K, V, Comp>::deserializeNode(FILE *f, Node *parentNode) {
    bool leaf = bp_tree_utils::readValLittle<bool>(f) != 0;
    auto s = bp_tree_utils::readValLittle<unsigned int>(f);
    if ((parentNode && s < minLoad) || s > order) {
        throw bp_tree_utils::stringFormat("Illegal keys size: size: %d, order: %d(offset: %ld)",
                                          s, order, ftell(f) - sizeof(int));
    }
    Node *node = new Node(initCap, leaf, comp);
    node->parentPtr = parentNode;
    for (unsigned int i = 0; i < s; ++i) {
        node->keys.push_back(bp_tree_utils::readValLittle<K>(f));
    }
    if (leaf) {
        for (unsigned int i = 0; i < s; ++i) {
            node->values.push_back(bp_tree_utils::readValLittle<V>(f));
        }
    } else {
        long offset;
        for (unsigned int i = 0; i < s; ++i) {
            long childOffset = bp_tree_utils::readValLittle<long>(f);
            offset = ftell(f);
            fseek(f, childOffset, SEEK_SET);
            node->childNodePtrs.push_back(deserializeNode(f, node));
            fseek(f, offset, SEEK_SET);
        }
    }

    return node;
}

template<typename K, typename V, class Comp>
BPTree<K, V, Comp>::~BPTree() {
    clear(root);
    delete root;
}

template<typename K, typename V, class Comp>
typename BPTree<K, V, Comp>::Node *BPTree<K, V, Comp>::getNextSibling(BPTree::Node *node) {
    if (!node) {
        return NULL;
    }
    if (!node->parentPtr) {
        return NULL;
    }
    K &key = node->keys.back();
    auto it = std::lower_bound(node->parentPtr->keys.begin(), node->parentPtr->keys.end(), key, comp);
    int pos = std::distance(node->parentPtr->keys.begin(), it);

    if (pos < node->parentPtr->keys.size() && comp(key, node->parentPtr->keys[pos]) == 0) {
        if (pos == node->parentPtr->keys.size() - 1) {
            Node *parentSibling = getNextSibling(node->parentPtr);
            if (parentSibling && !parentSibling->childNodePtrs.empty()) {
                return parentSibling->childNodePtrs[0];
            } else if (!parentSibling) {
                return NULL;
            }
        } else {
            return node->parentPtr->childNodePtrs[pos + 1];
        }
    }

    throw bp_tree_utils::stringFormat("the structure of the bp tree is not correct");
}


template<typename K, typename V, class Comp>
void BPTree<K, V, Comp>::recoverLinkList(bool valid) {
    Node *node = getFirstLeaf();
    K *lastKey = NULL;
    int count = 0;
    int s;
    while (node) {
        s = node->values.size();
        for (int i = 0; i < s; ++i) {
            count++;
            if (valid) {
                if (lastKey) {
                    const K &key = *lastKey;
                    if (comp(node->keys[i], key) <= 0)
                        throw std::string("invalid bp tree struct");
                }
                lastKey = &(node->keys[i]);
            }
        }
        node = node->next;
    }

    if (valid && count != size) {
        throw bp_tree_utils::stringFormat("Wrong size: expected %d but got %d", size, count);
    }
}

template<typename K, typename V, class Comp>
typename BPTree<K, V, Comp>::Node *BPTree<K, V, Comp>::copyNode(Node *parentPtr, Node *fromPtr) {
    if (!fromPtr) return NULL;
    Node *newNode = new Node(initCap, fromPtr->leaf, comp);
    newNode->parentPtr = parentPtr;
    newNode->keys = fromPtr->keys;

    if (fromPtr->leaf) {
        newNode->values = fromPtr->values;
    } else {
        for (Node *child : fromPtr->childNodePtrs) {
            newNode->childNodePtrs.push_back(copyNode(newNode, child));
        }
    }

    return newNode;
}

#endif //BPTREE_BPTREE_H
