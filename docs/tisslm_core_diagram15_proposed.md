# Proposed `Matrix` with Dynamic Shape Inference
**File:** `docs/tisslm_core_diagram15_proposed.md`

**Objective:** To diagram a `Matrix` class where the shape is not stored explicitly but is inferred dynamically from a fixed-size `4x4` sparse storage structure.

---

### 1. Proposed Core Data Structure

The `Matrix` no longer stores its shape. Instead, it holds a fixed `4x4` grid where elements can be "missing" (represented by `std::optional`). The default constructor initializes this `4x4` grid with all elements missing.

```
+------------------------------------------------------+
| Proposed `Matrix` Class                              |
|------------------------------------------------------|
|                                                      |
| private:                                             |
|   // A fixed 4x4 grid holding optional float values. |
|   std::vector<std::vector<std::optional<float>>> data_;
|                                                      |
| public:                                              |
|   Matrix(); // Creates a 4x4 grid of nullopt         |
|   std::vector<size_t> shape(); // Calculates shape    |
|   ...                                                |
|                                                      |
+------------------------------------------------------+
```

**Default Constructed Object State:**
```
data_ = {
  { nullopt, nullopt, nullopt, nullopt },
  { nullopt, nullopt, nullopt, nullopt },
  { nullopt, nullopt, nullopt, nullopt },
  { nullopt, nullopt, nullopt, nullopt }
}
```

---

### 2. Data Flow: Dynamic Shape Calculation

The `.shape()` method is no longer a simple accessor. It becomes an active computation that must be run each time the shape is needed.

**Example Call:** `my_matrix.shape();`

```
+-------------------+
|  `Matrix` object  |
| (with its `data_`) |
+-------------------+
        |
        v
+--------------------------------+
|   Dynamic Shape Calculation    |
+--------------------------------+
        |
        |
        v
+--------------------------------+
| 1. Determine Row Count (`dim0`)|
|   (Iterate rows 0-3. Count how |
|    many rows have at least one |
|    element that is not nullopt)|
+--------------------------------+
        |
        |
        v
+--------------------------------+
| 2. Determine Col Count (`dim1`)|
|  (For each non-empty row, find |
|   the index of the last non-   |
|   nullopt element. `dim1` is   |
|   the maximum of these indices)|
+--------------------------------+
        |
        |
        v
+--------------------------------+
|    Return calculated shape     |
|       (e.g., {2, 3})           |
+--------------------------------+
```

---

### 3. Data Flow: Element-wise Addition (`A + B`)

All operations must now handle "missing" elements on the fly, making the logic conditional at the element level.

```
+----------+          +----------+
| Matrix A |          | Matrix B |
+----------+          +----------+
     |                     |
     +----------+----------+
                |
                v
+--------------------------------+
|   Element-wise Add Operation   |
+--------------------------------+
                |
                v
+--------------------------------+
|    1. Create `result` Matrix   |
|  (A new 4x4 grid of nullopt)   |
+--------------------------------+
                |
                v
+--------------------------------+
|   2. Loop over entire 4x4 grid |
|    (from r=0 to 3, c=0 to 3)   |
+--------------------------------+
                |
                |
                v
+--------------------------------+
| 3. For each element (r, c):    |
|                                |
|   if (A[r][c].has_value() &&   |
|       B[r][c].has_value()) {   |
|                                |
|     result[r][c] =             |
|       A[r][c] + B[r][c];       |
|   }                            |
+--------------------------------+
                |
                v
+--------------------------------+
|   `result` Matrix returned     |
|  (Contains sums where both     |
|   operands had values)         |
+--------------------------------+
```
This design achieves the goal of inferring shape from structure. An operation equivalent to "reshape" would involve manually moving `std::optional<float>` values within the `data_` grid.