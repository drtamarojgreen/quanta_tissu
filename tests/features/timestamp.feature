Feature: Timestamp Data Type
  As a user, I want to be able to store and query TIMESTAMP data types in TissDB.

  Scenario: Insert and select a TIMESTAMP value
    Given a running TissDB instance
    And a collection named "logs" exists
    When I execute the TissQL query "INSERT INTO logs (level, event_time) VALUES ('INFO', TIMESTAMP '2024-07-27T10:00:00.123Z')" on "logs"
    Then the query should succeed
    When I execute the TissQL query "SELECT level, event_time FROM logs WHERE level = 'INFO'" on "logs"
    Then the query result should have 1 document
    And the document should contain the field "event_time" with the timestamp "2024-07-27T10:00:00.123000Z"

  Scenario: Filter records using a WHERE clause with TIMESTAMP
    Given a running TissDB instance
    And a collection named "sensor_data" exists
    When I execute the TissQL query "INSERT INTO sensor_data (sensor_id, reading, ts) VALUES ('A', 1.0, TIMESTAMP '2024-07-27T10:00:00Z')" on "sensor_data"
    And I execute the TissQL query "INSERT INTO sensor_data (sensor_id, reading, ts) VALUES ('B', 2.0, TIMESTAMP '2024-07-27T10:05:00Z')" on "sensor_data"
    And I execute the TissQL query "INSERT INTO sensor_data (sensor_id, reading, ts) VALUES ('C', 3.0, TIMESTAMP '2024-07-27T10:10:00Z')" on "sensor_data"
    Then the query should succeed
    When I execute the TissQL query "SELECT sensor_id FROM sensor_data WHERE ts > TIMESTAMP '2024-07-27T10:02:00Z'" on "sensor_data"
    Then the query result should have 2 documents
    And the result should contain a document with the field "sensor_id" having the value "B"
    And the result should contain a document with the field "sensor_id" having the value "C"

  Scenario: Create an index on a TIMESTAMP field
    Given a running TissDB instance
    And a collection named "metrics" exists
    When I create a timestamp index on the field "timestamp" in collection "metrics"
    Then the index should be created successfully


  Scenario: Filter records using BETWEEN and INTERVAL on TIMESTAMP
    Given a running TissDB instance
    And a collection named "temporal_events" exists
    When I execute the TissQL query "INSERT INTO temporal_events (name, ts) VALUES ('A', TIMESTAMP '2024-07-27T10:00:00Z')" on "temporal_events"
    And I execute the TissQL query "INSERT INTO temporal_events (name, ts) VALUES ('B', TIMESTAMP '2024-07-27T10:05:00Z')" on "temporal_events"
    And I execute the TissQL query "INSERT INTO temporal_events (name, ts) VALUES ('C', TIMESTAMP '2024-07-27T10:10:00Z')" on "temporal_events"
    Then the query should succeed
    When I execute the TissQL query "SELECT name FROM temporal_events WHERE ts BETWEEN TIMESTAMP '2024-07-27T10:02:00Z' AND TIMESTAMP '2024-07-27T10:09:00Z'" on "temporal_events"
    Then the query result should have 1 document
    And the result should contain a document with the field "name" having the value "B"
    When I execute the TissQL query "SELECT name FROM temporal_events WHERE ts + INTERVAL 5 MINUTES > TIMESTAMP '2024-07-27T10:12:00Z'" on "temporal_events"
    Then the query result should have 1 document
    And the result should contain a document with the field "name" having the value "C"

  Scenario: Use DATE and TIME literals and functions in WHERE clauses
    Given a running TissDB instance
    And a collection named "calendar_events" exists
    When I execute the TissQL query "INSERT INTO calendar_events (name, event_date, event_time, ts) VALUES ('Morning', DATE '2024-07-27', TIME '09:15:00', TIMESTAMP '2024-07-27T09:15:00Z')" on "calendar_events"
    And I execute the TissQL query "INSERT INTO calendar_events (name, event_date, event_time, ts) VALUES ('Noon', DATE '2024-07-27', TIME '12:00:00', TIMESTAMP '2024-07-27T12:00:00Z')" on "calendar_events"
    Then the query should succeed
    When I execute the TissQL query "SELECT name FROM calendar_events WHERE event_date = DATE '2024-07-27' AND event_time = TIME '09:15:00'" on "calendar_events"
    Then the query result should have 1 document
    And the result should contain a document with the field "name" having the value "Morning"
    When I execute the TissQL query "SELECT name FROM calendar_events WHERE EXTRACT(HOUR FROM ts) >= 12 AND DATE(ts) = DATE '2024-07-27' AND TIME(ts) >= TIME '12:00:00'" on "calendar_events"
    Then the query result should have 1 document
    And the result should contain a document with the field "name" having the value "Noon"


  Scenario: Use EXTRACT and NOW in TIMESTAMP filtering
    Given a running TissDB instance
    And a collection named "audit_logs" exists
    When I execute the TissQL query "INSERT INTO audit_logs (actor, ts) VALUES ('system', TIMESTAMP '2024-07-27T12:00:00Z')" on "audit_logs"
    And I execute the TissQL query "INSERT INTO audit_logs (actor, ts) VALUES ('admin', TIMESTAMP '2024-07-27T23:15:00Z')" on "audit_logs"
    Then the query should succeed
    When I execute the TissQL query "SELECT actor FROM audit_logs WHERE EXTRACT(HOUR FROM ts) >= 20" on "audit_logs"
    Then the query result should have 1 document
    And the result should contain a document with the field "actor" having the value "admin"
    When I execute the TissQL query "SELECT actor FROM audit_logs WHERE ts <= NOW()" on "audit_logs"
    Then the query result should have 2 documents
