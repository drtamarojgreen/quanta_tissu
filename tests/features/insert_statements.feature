Feature: TissQL INSERT statements

  Scenario: A valid INSERT statement
    Given a TissDB instance
    And the collection "users" is empty
    When I execute the TissQL query "INSERT INTO users (name, age, city) VALUES ('Alice', 30, 'New York');"
    Then the query should succeed
    And the collection "users" should contain a document with "name" = "Alice"

  Scenario: An INSERT statement with various data types
    Given a TissDB instance
    And the collection "products" is empty
    When I execute the TissQL query "INSERT INTO products (name, price, in_stock, description) VALUES ('Laptop', 1200.50, true, NULL);"
    Then the query should succeed
    And the collection "products" should contain a document with "name" = "Laptop", "price" = 1200.50, "in_stock" = true, and "description" = NULL

  Scenario: An INSERT statement with a syntax error
    Given a TissDB instance
    When I execute the TissQL query "INSERT users (name) VALUES ('Bob');"
    Then the query should fail with a syntax error

  Scenario: An INSERT statement with mismatched columns and values
    Given a TissDB instance
    When I execute the TissQL query "INSERT INTO users (name, age) VALUES ('Charlie');"
    Then the query should fail with a mismatched column/value count error

  Scenario: An INSERT statement into a non-existent collection
    Given a TissDB instance
    When I execute the TissQL query "INSERT INTO non_existent_collection (data) VALUES ('some data');"
    Then the query should succeed and create the collection
