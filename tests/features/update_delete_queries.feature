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

  Scenario Teardown:
    When I delete the collection "products"
    Then the collection "products" should not exist
