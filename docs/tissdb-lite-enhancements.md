# TissDB-Lite Enhancements

This document outlines the key enhancements made to `tissdb-lite` to improve its functionality, robustness, and adherence to a "from scratch" development philosophy.

## 1. Removal of External Dependencies

In line with the goal of building `tissdb-lite` from scratch, external dependencies have been minimized. Specifically:

*   **`uuid` library replaced:** The reliance on the `uuid` npm package for generating unique identifiers has been removed. A custom, lightweight `_generateId` method is now used, combining a timestamp with a random string to produce reasonably unique IDs. This reduces the project's footprint and external reliance.

## 2. Enhanced Query Engine

The query engine has been significantly upgraded to provide more powerful and flexible querying capabilities for both in-memory collections and Wix Data collections.

*   **Advanced Query Parsing:** The `parseCondition` method (and its internal helpers `_tokenize`, `_toRpn`, `_evaluateOperator`) has been refactored to implement a robust shunting-yard algorithm. This allows `tissdb-lite` to correctly parse and evaluate complex query strings that include:
    *   Multiple logical operators (`AND`, `OR`).
    *   Comparison operators (`=`, `!=`, `>`, `>=`, `<`, `<=`).
    *   Parentheses for grouping conditions and controlling operator precedence.

*   **Unified Query Language:** The same TissLang-like query string syntax can now be used for both:
    *   **In-memory `find` operations:** The `find` method for in-memory collections now leverages the new query parsing logic, enabling complex filtering directly on the ephemeral data.
    *   **Wix Data `executeQuery` operations:** The `parseCondition` method translates the complex query string into the appropriate `wix-data.WixDataFilter` object, ensuring seamless integration with Wix's persistent data storage.

## 3. Self-Contained Testing

To ensure the reliability of the enhanced functionality without introducing external testing frameworks, a self-contained test suite has been implemented.

*   **`test.js` file:** A dedicated `test.js` file has been created within the `lite/` directory. This file imports the `TissDBLite` class and performs a series of assertions against its methods.
*   **Manual Assertion System:** A simple `assert` function is used to check conditions and report successes or failures directly to the console. This provides immediate feedback on the library's correctness.
*   **`package.json` Integration:** The `test` script in `package.json` has been updated to execute `node lite/test.js`, making it easy to run the verification suite.

These enhancements collectively make `tissdb-lite` a more capable and self-sufficient library, offering advanced data interaction features while maintaining a minimal and controlled dependency footprint.
