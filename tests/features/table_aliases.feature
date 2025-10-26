Feature: TissQL Table Aliases

  Background:
    Given a running TissDB instance
    When I delete the collection "orders"
    And I delete the collection "customers"
    And a collection named "orders" exists
    And a collection named "customers" exists
    And I insert the following documents into "customers":
      | _id   | name    | city      |
      | cust1 | Alice   | New York  |
      | cust2 | Bob     | London    |
      | cust3 | Charlie | Paris     |
    And I insert the following documents into "orders":
      | order_id | customer_id | item     | amount |
      | ord1     | cust1       | Laptop   | 1200   |
      | ord2     | cust2       | Mouse    | 25     |
      | ord3     | cust1       | Keyboard | 75     |

  Scenario: Select with a simple table alias
    When I execute the TissQL query "SELECT o.item FROM orders AS o WHERE o.amount > 100"
    Then the query result should have 2 documents
    And the query result should contain a document with "o.item" = "Laptop"
    And the query result should contain a document with "o.item" = "Keyboard"

  Scenario: Select with JOIN and table aliases
    When I execute the TissQL query "SELECT c.name, o.item FROM orders AS o JOIN customers AS c ON o.customer_id = c._id"
    Then the query result should have 3 documents
    And the query result should contain a document with "c.name" = "Alice" and "o.item" = "Laptop"
    And the query result should contain a document with "c.name" = "Bob" and "o.item" = "Mouse"
    And the query result should contain a document with "c.name" = "Alice" and "o.item" = "Keyboard"

  Scenario: Select with JOIN and table aliases (without AS)
    When I execute the TissQL query "SELECT c.name, o.item FROM orders o JOIN customers c ON o.customer_id = c._id"
    Then the query result should have 3 documents
    And the query result should contain a document with "c.name" = "Alice" and "o.item" = "Laptop"
    And the query result should contain a document with "c.name" = "Bob" and "o.item" = "Mouse"
    And the query result should contain a document with "c.name" = "Alice" and "o.item" = "Keyboard"

  Scenario: Select with JOIN and mixed aliases and full names
    When I execute the TissQL query "SELECT customers.name, o.item FROM orders o JOIN customers ON o.customer_id = customers._id"
    Then the query result should have 3 documents
    And the query result should contain a document with "customers.name" = "Alice" and "o.item" = "Laptop"
