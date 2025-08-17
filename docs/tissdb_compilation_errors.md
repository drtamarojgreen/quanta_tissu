# TissDB Compilation and Test-Time Issues

This document outlines the issues found and fixed during the attempt to get the `tissdb` tests to compile and run successfully.

## Summary

The initial task was to fix compilation errors in the `tissdb` tests. This was successful, but it revealed further issues, including linker errors and runtime failures (segmentation faults) during testing.

The compilation and linker errors were all resolved. The root cause of the final segmentation fault has been identified as a series of critical memory management bugs in the custom `tissdb/storage/List.h` class.

As per instructions, this class has **not** been refactored due to the high risk of introducing subtle bugs. Instead, this document details the issues found.

## 1. Initial Compilation Errors

The first set of errors were simple compilation errors in the test files:
-   `tests/db/test_executor.cpp`: Use of an undeclared variable `updated_doc_opt`. This was fixed by adding the necessary variable declaration and initialization.
-   `tests/db/test_transactions.cpp`: Incorrectly accessing a member of a `std::shared_ptr` instead of the object it points to. This was fixed by correctly dereferencing the pointer. There was also a copy-paste error in an assertion which was corrected.

## 2. Linker Errors

After fixing the initial compilation errors, the build failed at the linking stage with numerous `undefined reference` errors.

**Cause:** The `tissdb/Makefile` was incomplete and did not include all the necessary source files from the `storage` and `common` directories. It was also missing the source files for the `tissu_sinew` component, which is a dependency of the tests.

**Fix:** The `Makefile` was updated to include all the required source files, which resolved all linker errors.

## 3. Runtime Errors and Segmentation Fault

After the build was successful, running the tests revealed a failing test and a segmentation fault.

### 3.1. `DocumentSerialization` Test Failure

**Symptom:** The `DocumentSerialization` test failed with an assertion error comparing `std::chrono::system_clock::time_point` objects.

**Cause:** The serialization code in `common/serialization.cpp` was truncating `time_point` objects to second-level precision, losing all sub-second precision.

**Fix:** The serialization and deserialization logic for `DateTime` was updated to use nanosecond precision, which is the highest precision available in `std::chrono`. This fixed the test failure.

### 3.2. Segmentation Fault and Memory Corruption in `List.h`

**Symptom:** Even after fixing the `DocumentSerialization` test, the test suite still crashed with a segmentation fault. The crash was preceded by error messages like `Error during SSTable scan: Attempted to read excessively large byte array...`, which pointed to data corruption.

**Root Cause:** The root cause of the segmentation fault is a series of critical memory management bugs in the custom container class `tissdb/storage/List.h`. This class is used by the `BPTree` implementation, and the crash was triggered by the `test_bpp_tree.cpp` tests.

The `List.h` class manually manages memory for a list of objects of type `T`. However, it does so in a way that is only safe for Plain Old Data (POD) types. When used with complex, non-trivial types like `std::string` (which is the case in `BPTree`), its behavior is undefined and leads to memory corruption.

The specific bugs are:

1.  **`malloc`/`free` without constructor/destructor calls:** The code uses `malloc` to allocate memory for the list elements but never calls their constructors. When the list is destroyed, it calls `free` but never calls the destructors of the elements. This is a guaranteed memory leak for types like `std::string` that manage their own memory.
    -   **Location:** `List::List()`, `List::~List()`, `List::initAllocate()`.

2.  **`memcpy`/`memmove` on non-trivial types:** The code uses `memcpy` and `memmove` to copy and move elements around. For `std::string`, this just copies the internal pointer, size, and capacity, not the actual character data. This leads to two `std::string` objects thinking they own the same memory, which results in double-frees and other memory corruption when one of them is destroyed or modified.
    -   **Location:** `List::List(const List&)`, `List::insert()`, `List::removeAt()`, `List::removeRange()`.

3.  **`realloc` on non-trivial types:** `realloc` might move the memory block to a new location. For non-trivial types, this is equivalent to a `memmove` followed by `free`ing the old block, without calling any destructors or copy/move constructors. This is undefined behavior and a major source of memory corruption.
    -   **Location:** `List::expandTo()`, `List::trimToSize()`.

**Recommendation:**

The `List.h` class needs a complete refactoring to handle object lifetimes correctly. The recommended solution is to replace it entirely with `std::vector`, which is a standard, well-tested, and memory-safe container that handles all of these cases correctly.

If replacing it is not feasible, the class must be manually fixed by:
-   Using placement new to construct objects in the allocated memory.
-   Explicitly calling destructors for elements before freeing memory.
-   Replacing `memcpy`/`memmove` with element-wise copy/move construction.
-   Replacing `realloc` with a proper new-copy-delete cycle.

**As instructed, these changes have not been implemented to avoid the risk of introducing further issues.** The code has been left in its current state, with the compilation and other runtime errors fixed. The segmentation fault will persist until `List.h` is properly refactored.