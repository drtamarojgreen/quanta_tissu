Feature: TissQL Select Queries

  Background:
    Given a running TissDB instance
    And a collection named "products" exists
    And I insert the following documents into "products":
      | id   | name         | category | price | stock |
      | p001 | Laptop       | Tech     | 1200  | 50    |
      | p002 | Keyboard     | Tech     | 75    | 200   |
      | p003 | Mouse        | Tech     | 25    | 500   |
      | p004 | T-Shirt      | Apparel  | 20    | 300   |
      | p005 | Coffee Mug   | Kitchen  | 15    | 1000  |

  Scenario: Select all data from a collection
    When I execute the TissQL query "SELECT * FROM products"
    Then the query result should have 5 documents
    And the query result should contain a document with "name" = "Laptop"
    And the query result should contain a document with "price" = 20

  Scenario: Select specific fields from a collection
    When I execute the TissQL query "SELECT name, price FROM products"
    Then the query result should have 5 documents
    And each document in the result should have the fields ["name", "price"]
    And each document in the result should not have the fields ["category", "stock"]
    And the query result should contain a document with "name" = "Keyboard" and "price" = 75

  Scenario: Filter data with a WHERE clause
    When I execute the TissQL query "SELECT * FROM products WHERE category = 'Tech'"
    Then the query result should have 3 documents
    And the query result should only contain documents where "category" is "Tech"

  Scenario: Select specific fields with a WHERE clause
    When I execute the TissQL query "SELECT name FROM products WHERE stock > 250"
    Then the query result should have 2 documents
    And the query result should contain a document with "name" = "Mouse"
    And the query result should contain a document with "name" = "T-Shirt"
    And the query result should not contain a document with "name" = "Laptop"

  Scenario: Complex query with multiple conditions
    When I execute the TissQL query "SELECT name, price FROM products WHERE category = 'Tech' AND price < 100"
    Then the query result should have 2 documents
    And the query result should contain a document with "name" = "Keyboard" and "price" = 75
    And the query result should contain a document with "name" = "Mouse" and "price" = 25
    And the query result should not contain a document with "name" = "Laptop"

  Scenario: Query with no results
    When I execute the TissQL query "SELECT * FROM products WHERE price > 2000"
    Then the query result should be empty

  Scenario: Sort by a numeric field in ascending order
    When I execute the TissQL query "SELECT name, price FROM products ORDER BY price ASC"
    Then the query result should have 5 documents
    And the documents should be in the following order for the key "name": ["Coffee Mug", "T-Shirt", "Mouse", "Keyboard", "Laptop"]

  Scenario: Sort by a numeric field in descending order
    When I execute the TissQL query "SELECT name, price FROM products ORDER BY price DESC"
    Then the query result should have 5 documents
    And the documents should be in the following order for the key "name": ["Laptop", "Keyboard", "Mouse", "T-Shirt", "Coffee Mug"]

  Scenario: Sort by a string field
    When I execute the TissQL query "SELECT name FROM products ORDER BY name"
    Then the query result should have 5 documents
    And the documents should be in the following order for the key "name": ["Coffee Mug", "Keyboard", "Laptop", "Mouse", "T-Shirt"]

  Scenario: Sort by multiple fields
    When I execute the TissQL query "SELECT name, category, price FROM products ORDER BY category ASC, price DESC"
    Then the query result should have 5 documents
    And the documents should be in the following order for the key "name": ["T-Shirt", "Coffee Mug", "Laptop", "Keyboard", "Mouse"]
