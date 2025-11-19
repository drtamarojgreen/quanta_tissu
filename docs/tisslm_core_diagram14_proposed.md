# Proposed `Matrix` Class with N-Dimensional Nested Vector Storage
**File:** `docs/tisslm_core_diagram14_proposed.md`

**Objective:** To propose and diagram a `Matrix` class design that uses nested, n-dimensional data storage instead of a flat 1D array, per the user's directive.

---

### 1. Proposed Core Data Structure

To handle a variable number of dimensions (N-dimensions), a recursive, tree-like structure is required. A `std::variant` can be used to define a node that is either a leaf (a `float`) or a branch (a vector of more nodes).

```
+------------------------------------------------------+
| struct TensorNode                                    |
|------------------------------------------------------|
|                                                      |
| std::variant<                                        |
|   float,                     // Leaf node value       |
|   std::vector<TensorNode>    // Branch node           |
| > data;                                              |
|                                                      |
+------------------------------------------------------+


+------------------------------------------------------+
| Proposed `Matrix` Class                              |
|------------------------------------------------------|
|                                                      |
| private:                                             |
|   std::vector<size_t> shape_;                        |
|   TensorNode root_;          // Root of the tree      |
|                                                      |
+------------------------------------------------------+
```

---

### 2. Data Flow: Construction

Construction becomes a recursive process of building the tree structure to match the desired shape.

**Example Call:** `Matrix my_matrix({2, 2});`

```
+-------------------+
|  `shape` vector   |
|  (Value: {2, 2})  |
+-------------------+
        |
        v
+--------------------------------+
|  Recursive Constructor Logic   |
+--------------------------------+
        |
        v
+--------------------------------+
|  Create `root_` node (Branch)  |
| (Contains a vector of 2 nodes) |
+--------------------------------+
        |
+----------------------------------+
|                                  |
v                                  v
+-----------------------+          +-----------------------+
| Create Node 1 (Branch)|          | Create Node 2 (Branch)|
| (Contains a vector of |          | (Contains a vector of |
|  2 leaf nodes)        |          |  2 leaf nodes)        |
+-----------------------+          +-----------------------+
        |                                  |
+-----------+-----------+          +-----------+-----------+
|           |           |          |           |           |
v           v           v          v           v           v
+-----------+ +-----------+        +-----------+ +-----------+
| Leaf Node | | Leaf Node |        | Leaf Node | | Leaf Node |
|  (float)  | |  (float)  |        |  (float)  | |  (float)  |
+-----------+ +-----------+        +-----------+ +-----------+

// The final `root_` object is a tree of nested vectors.
```

---

### 3. Data Flow: Element Access

Element access requires recursively traversing the tree structure.

**Example Call:** `my_matrix({1, 0});`

```
+-------------------+
| `indices` vector  |
|  (Value: {1, 0})  |
+-------------------+
        |
        v
+--------------------------------+
|  Recursive `operator()` Logic  |
+--------------------------------+
        |
        v
+--------------------------------+
|    Access `root_` node's data  |
|    (Get vector of nodes)       |
+--------------------------------+
        | (Select element at index 1)
        v
+--------------------------------+
| Access child node `[1]`'s data |
|    (Get vector of nodes)       |
+--------------------------------+
        | (Select element at index 0)
        v
+--------------------------------+
| Access leaf node `[1][0]`'s data|
|       (Get float value)        |
+--------------------------------+
        |
        v
+-------------------+
|  Returned `float` |
+-------------------+
```

---

### 4. Data Flow: Reshape Operation (Illustrates Inefficiency)

Reshaping the matrix is no longer a simple metadata change. It requires flattening the entire nested structure and rebuilding it from scratch.

**Example Call:** `my_matrix.reshape({4});`

```
+-------------------+
|  `Matrix` object  |
|   (A [2,2] tree)  |
+-------------------+
        |
        v
+--------------------------------+
| 1. Recursive Flatten Operation |
|   (Traverse entire tree to    |
|    extract all float values)   |
+--------------------------------+
        |
        v
+--------------------------------+
| Temporary `std::vector<float>` |
| (Value: {f1, f2, f3, f4})      |
+--------------------------------+
        |
        v
+--------------------------------+
|   2. Recursive Construction    |
| (Use logic from Part 2 with    |
|  new shape `{4}` and temp data)|
+--------------------------------+
        |
        v
+--------------------------------+
|    New `Matrix` object         |
|      (A new [4] tree)          |
+--------------------------------+
```