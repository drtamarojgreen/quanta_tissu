Feature: Date and Time Data Types
  As a user, I want to be able to store and query DATE, TIME, and DATETIME data types in TissDB.

  Scenario: Insert and select a DATE value
    Given a database "testdb"
    And a collection "events"
    When I execute the query "INSERT INTO events (event_name, event_date) VALUES ('New Year', DATE '2024-01-01')"
    Then the query should succeed
    When I execute the query "SELECT event_name, event_date FROM events WHERE event_name = 'New Year'"
    Then the result should contain 1 document
    And the document should contain the field "event_date" with the date "2024-01-01"

  Scenario: Insert and select a TIME value
    Given a database "testdb"
    And a collection "appointments"
    When I execute the query "INSERT INTO appointments (title, app_time) VALUES ('Lunch', TIME '12:30:00')"
    Then the query should succeed
    When I execute the query "SELECT title, app_time FROM appointments WHERE title = 'Lunch'"
    Then the result should contain 1 document
    And the document should contain the field "app_time" with the time "12:30:00"

  Scenario: Insert and select a DATETIME value
    Given a database "testdb"
    And a collection "logs"
    When I execute the query "INSERT INTO logs (level, log_time) VALUES ('INFO', DATETIME '2024-07-26 10:00:00')"
    Then the query should succeed
    When I execute the query "SELECT level, log_time FROM logs WHERE level = 'INFO'"
    Then the result should contain 1 document
    And the document should contain the field "log_time" with the datetime "2024-07-26 10:00:00"

  Scenario: Filter records using a WHERE clause with DATE
    Given a database "testdb"
    And a collection "history"
    When I execute the query "INSERT INTO history (event, event_date) VALUES ('Event A', DATE '2023-12-31')"
    And I execute the query "INSERT INTO history (event, event_date) VALUES ('Event B', DATE '2024-01-15')"
    And I execute the query "INSERT INTO history (event, event_date) VALUES ('Event C', DATE '2024-02-01')"
    Then the query should succeed
    When I execute the query "SELECT event FROM history WHERE event_date > DATE '2024-01-01'"
    Then the result should contain 2 documents
    And the result should contain a document with the field "event" having the value "Event B"
    And the result should contain a document with the field "event" having the value "Event C"
