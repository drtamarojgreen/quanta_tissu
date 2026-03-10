Feature: Date and Time Data Types
  As a user, I want to be able to store and query DATE, TIME, and DATETIME data types in TissDB.

  Scenario: Insert and select a DATE value
    Given a running TissDB instance
    And a collection named "events" exists
    When I execute the TissQL query "INSERT INTO events (event_name, event_date) VALUES ('New Year', DATE '2024-01-01')" on "events"
    Then the query should succeed
    When I execute the TissQL query "SELECT event_name, event_date FROM events WHERE event_name = 'New Year'" on "events"
    Then the query result should have 1 document
    And the document should contain the field "event_date" with the date "2024-01-01"

  Scenario: Insert and select a TIME value
    Given a running TissDB instance
    And a collection named "appointments" exists
    When I execute the TissQL query "INSERT INTO appointments (title, app_time) VALUES ('Lunch', TIME '12:30:00')" on "appointments"
    Then the query should succeed
    When I execute the TissQL query "SELECT title, app_time FROM appointments WHERE title = 'Lunch'" on "appointments"
    Then the query result should have 1 document
    And the document should contain the field "app_time" with the time "12:30:00"

  Scenario: Insert and select a DATETIME value
    Given a running TissDB instance
    And a collection named "logs" exists
    When I execute the TissQL query "INSERT INTO logs (level, log_time) VALUES ('INFO', DATETIME '2024-07-26 10:00:00')" on "logs"
    Then the query should succeed
    When I execute the TissQL query "SELECT level, log_time FROM logs WHERE level = 'INFO'" on "logs"
    Then the query result should have 1 document
    And the document should contain the field "log_time" with the datetime "2024-07-26 10:00:00"

  Scenario: Filter records using a WHERE clause with DATE
    Given a running TissDB instance
    And a collection named "history" exists
    When I execute the TissQL query "INSERT INTO history (event, event_date) VALUES ('Event A', DATE '2023-12-31')" on "history"
    And I execute the TissQL query "INSERT INTO history (event, event_date) VALUES ('Event B', DATE '2024-01-15')" on "history"
    And I execute the TissQL query "INSERT INTO history (event, event_date) VALUES ('Event C', DATE '2024-02-01')" on "history"
    Then the query should succeed
    When I execute the TissQL query "SELECT event FROM history WHERE event_date > DATE '2024-01-01'" on "history"
    Then the query result should have 2 documents
    And the result should contain a document with the field "event" having the value "Event B"
    And the result should contain a document with the field "event" having the value "Event C"


  Scenario: Filter records using DATETIME with range comparisons
    Given a running TissDB instance
    And a collection named "timeline" exists
    When I execute the TissQL query "INSERT INTO timeline (name, dt) VALUES ('Start', DATETIME '2024-07-26 09:00:00')" on "timeline"
    And I execute the TissQL query "INSERT INTO timeline (name, dt) VALUES ('Middle', DATETIME '2024-07-26 10:30:00')" on "timeline"
    And I execute the TissQL query "INSERT INTO timeline (name, dt) VALUES ('End', DATETIME '2024-07-26 12:00:00')" on "timeline"
    Then the query should succeed
    When I execute the TissQL query "SELECT name FROM timeline WHERE dt >= DATETIME '2024-07-26 10:00:00' AND dt < DATETIME '2024-07-26 12:00:00'" on "timeline"
    Then the query result should have 1 document
    And the result should contain a document with the field "name" having the value "Middle"

  Scenario: Filter records using DATE and TIME ranges
    Given a running TissDB instance
    And a collection named "schedule" exists
    When I execute the TissQL query "INSERT INTO schedule (event, event_date, event_time) VALUES ('Morning Run', DATE '2024-07-27', TIME '06:30:00')" on "schedule"
    And I execute the TissQL query "INSERT INTO schedule (event, event_date, event_time) VALUES ('Lunch', DATE '2024-07-27', TIME '12:30:00')" on "schedule"
    And I execute the TissQL query "INSERT INTO schedule (event, event_date, event_time) VALUES ('Dinner', DATE '2024-07-28', TIME '19:00:00')" on "schedule"
    Then the query should succeed
    When I execute the TissQL query "SELECT event FROM schedule WHERE event_date = DATE '2024-07-27' AND event_time > TIME '10:00:00'" on "schedule"
    Then the query result should have 1 document
    And the result should contain a document with the field "event" having the value "Lunch"


  Scenario: Delete records using DATE and TIME predicates
    Given a running TissDB instance
    And a collection named "cleanup_schedule" exists
    When I execute the TissQL query "INSERT INTO cleanup_schedule (event, event_date, event_time) VALUES ('Keep', DATE '2024-07-27', TIME '09:00:00')" on "cleanup_schedule"
    And I execute the TissQL query "INSERT INTO cleanup_schedule (event, event_date, event_time) VALUES ('DeleteMe', DATE '2024-07-27', TIME '18:00:00')" on "cleanup_schedule"
    Then the query should succeed
    When I execute the TissQL query "DELETE FROM cleanup_schedule WHERE event_date = DATE '2024-07-27' AND event_time >= TIME '12:00:00'" on "cleanup_schedule"
    Then the query should succeed
    When I execute the TissQL query "SELECT event FROM cleanup_schedule" on "cleanup_schedule"
    Then the query result should have 1 document
    And the result should contain a document with the field "event" having the value "Keep"
