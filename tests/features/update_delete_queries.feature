Feature: TissDB UPDATE and DELETE Query Operations

  Background:
    Given a running TissDB instance
    And a collection named "products" exists
    And I insert the following documents into "products":
      | id    | content                                            |
      | prod1 | {"name": "Laptop", "category": "Electronics", "price": 1200} |
      | prod2 | {"name": "Keyboard", "category": "Electronics", "price": 75}   |
      | prod3 | {"name": "Mouse", "category": "Electronics", "price": 25}    |
      | prod4 | {"name": "Desk Chair", "category": "Furniture", "price": 150}  |
      | prod5 | {"name": "Desk Lamp", "category": "Furniture", "price": 35}    |

  Scenario: Update a single document using a WHERE clause
    When I execute the TissQL query "UPDATE products SET price = 1250 WHERE name = 'Laptop'" on "products"
    Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1250}
    And the document with ID "prod2" in "products" should have content {"name": "Keyboard", "category": "Electronics", "price": 75}

  Scenario: Update multiple documents using a WHERE clause
    When I execute the TissQL query "UPDATE products SET price = price + 5 WHERE category = 'Furniture'" on "products"
    Then the document with ID "prod4" in "products" should have content {"name": "Desk Chair", "category": "Furniture", "price": 155}
    And the document with ID "prod5" in "products" should have content {"name": "Desk Lamp", "category": "Furniture", "price": 40}
    And the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1200}

  Scenario: Update with a WHERE clause that matches no documents
    When I execute the TissQL query "UPDATE products SET category = 'Office Supplies' WHERE price > 2000" on "products"
    Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1200}
    And the document with ID "prod2" in "products" should have content {"name": "Keyboard", "category": "Electronics", "price": 75}
    And the document with ID "prod3" in "products" should have content {"name": "Mouse", "category": "Electronics", "price": 25}
    And the document with ID "prod4" in "products" should have content {"name": "Desk Chair", "category": "Furniture", "price": 150}
    And the document with ID "prod5" in "products" should have content {"name": "Desk Lamp", "category": "Furniture", "price": 35}

  Scenario: Delete a single document using a WHERE clause
    When I execute the TissQL query "DELETE FROM products WHERE name = 'Keyboard'" on "products"
    Then the document with ID "prod2" in "products" should not exist
    And the document with ID "prod1" in "products" should exist

  Scenario: Delete multiple documents using a WHERE clause
    When I execute the TissQL query "DELETE FROM products WHERE category = 'Furniture'" on "products"
    Then the document with ID "prod4" in "products" should not exist
    And the document with ID "prod5" in "products" should not exist
    And the document with ID "prod1" in "products" should exist

  Scenario: Delete with a WHERE clause that matches no documents
    When I execute the TissQL query "DELETE FROM products WHERE price > 2000" on "products"
    Then the document with ID "prod1" in "products" should exist
    And the document with ID "prod2" in "products" should exist
    And the document with ID "prod3" in "products" should exist
    And the document with ID "prod4" in "products" should exist
    And the document with ID "prod5" in "products" should exist

  Scenario: Update with subtraction
    When I execute the TissQL query "UPDATE products SET price = price - 10 WHERE name = 'Desk Chair'" on "products"
    Then the document with ID "prod4" in "products" should have content {"name": "Desk Chair", "category": "Furniture", "price": 140}

  Scenario: Update with multiplication
    When I execute the TissQL query "UPDATE products SET price = price * 1.1 WHERE category = 'Electronics'" on "products"
    Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1320}
    And the document with ID "prod2" in "products" should have content {"name": "Keyboard", "category": "Electronics", "price": 82.5}
    And the document with ID "prod3" in "products" should have content {"name": "Mouse", "category": "Electronics", "price": 27.5}

  Scenario: Update with division
    When I execute the TissQL query "UPDATE products SET price = price / 2 WHERE name = 'Laptop'" on "products"
    Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 600}

  Scenario: Update multiple fields with expressions
    When I execute the TissQL query "UPDATE products SET price = price + 100, name = 'Gaming Laptop' WHERE id = 'prod1'" on "products"
    Then the document with ID "prod1" in "products" should have content {"name": "Gaming Laptop", "category": "Electronics", "price": 1300}

  Scenario Teardown:
    When I delete the collection "products"
    Then the collection "products" should not exist
