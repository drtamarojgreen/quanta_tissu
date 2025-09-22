# TissDB-Lite API Reference

This document provides a comprehensive API reference for the `TissDBLite` class, detailing its methods, parameters, and return values.

## `TissDBLite` Class

The `TissDBLite` class provides functionalities for managing in-memory collections and translating TissLang queries to Wix Data filters.

### Constructor

`new TissDBLite()`

Initializes a new instance of the `TissDBLite` class.

### Methods

#### `createCollection(name)`

Creates an in-memory collection.

*   **Parameters:**
    *   `name` (string): The name of the collection to create.
*   **Returns:** `void`
*   **Throws:** `Error` if the collection already exists (though current implementation allows overwriting).

#### `insert(collectionName, item)`

Inserts a single item into an in-memory collection. Automatically generates `_id`, `_createdDate`, and `_updatedDate` fields.

*   **Parameters:**
    *   `collectionName` (string): The name of the collection to insert into.
    *   `item` (object): The item (document) to insert.
*   **Returns:** `object` - The inserted item with generated fields.
*   **Throws:** `Error` if the specified `collectionName` does not exist.

#### `bulkInsert(collectionName, items)`

Inserts multiple items into an in-memory collection. Automatically generates `_id`, `_createdDate`, and `_updatedDate` fields for each item.

*   **Parameters:**
    *   `collectionName` (string): The name of the collection to insert into.
    *   `items` (Array<object>): An array of items (documents) to insert.
*   **Returns:** `Array<object>` - An array of the inserted items with generated fields.
*   **Throws:** `Error` if the specified `collectionName` does not exist.

#### `find(collectionName, condition_string)`

Finds items in an in-memory collection using a TissLang-like query string.

*   **Parameters:**
    *   `collectionName` (string): The name of the collection to search within.
    *   `condition_string` (string, optional): The query string for filtering. Supports `AND`, `OR`, comparison operators (`=`, `!=`, `>`, `>=`, `<`, `<=`), and parentheses. If omitted, all items in the collection are returned.
*   **Returns:** `Array<object>` - An array of items that match the `condition_string`.
*   **Throws:** `Error` if the specified `collectionName` does not exist or if the `condition_string` is malformed.

#### `update(collectionName, item)`

Updates an item in an in-memory collection. The `item` must contain an `_id` field to identify the document to update. The `_updatedDate` field is automatically updated.

*   **Parameters:**
    *   `collectionName` (string): The name of the collection containing the item.
    *   `item` (object): The item (document) with updated fields. Must include the `_id` of the item to be updated.
*   **Returns:** `object` - The updated item.
*   **Throws:** `Error` if the specified `collectionName` does not exist or if an item with the given `_id` is not found.

#### `remove(collectionName, itemId)`

Removes an item from an in-memory collection by its ID.

*   **Parameters:**
    *   `collectionName` (string): The name of the collection from which to remove the item.
    *   `itemId` (string): The `_id` of the item to remove.
*   **Returns:** `void`
*   **Throws:** `Error` if the specified `collectionName` does not exist.

#### `exportCollection(collectionName)`

Exports all items from an in-memory collection. Returns a deep copy of the collection to prevent external modification of the internal state.

*   **Parameters:**
    *   `collectionName` (string): The name of the collection to export.
*   **Returns:** `Array<object>` - An array containing all items in the collection.
*   **Throws:** `Error` if the specified `collectionName` does not exist.

#### `executeQuery(command)`

Executes a TissLang `QUERY` command against a Wix Data collection. This method translates the `condition_string` within the command into a `wix-data` filter and executes the query against Wix Data.

*   **Parameters:**
    *   `command` (object): An object representing the parsed TissLang `QUERY` command. Expected structure: `{ collection: string, condition_string: string }`.
*   **Returns:** `Promise<Array<object>>` - A promise that resolves to an array of items retrieved from the Wix Data collection.
*   **Throws:** `Error` if the `condition_string` is malformed or if there are issues with the Wix Data query.
