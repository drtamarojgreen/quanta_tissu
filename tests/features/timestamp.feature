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
