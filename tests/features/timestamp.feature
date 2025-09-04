Feature: Timestamp Data Type
  As a user, I want to be able to store and query ISO 8601 formatted timestamps in TissDB.

  Scenario: Insert and select a Timestamp value
    Given a running TissDB instance
    And a collection named "logs" exists
    When I execute the TissQL query "INSERT INTO logs (level, event_time) VALUES ('INFO', '2025-01-15T10:00:00Z')" on "logs"
    Then the query should succeed
    When I execute the TissQL query "SELECT level, event_time FROM logs WHERE level = 'INFO'" on "logs"
    Then the query result should have 1 document
    And the document should contain the field "event_time" with the timestamp "2025-01-15T10:00:00Z"

  Scenario: Filter records using an exact timestamp match
    Given a running TissDB instance
    And a collection named "audits" exists
    When I execute the TissQL query "INSERT INTO audits (action, event_time) VALUES ('LOGIN_SUCCESS', '2025-02-10T09:00:00Z')" on "audits"
    And I execute the TissQL query "INSERT INTO audits (action, event_time) VALUES ('LOGIN_FAILURE', '2025-02-10T09:00:05Z')" on "audits"
    When I execute the TissQL query "SELECT action FROM audits WHERE event_time = '2025-02-10T09:00:00Z'" on "audits"
    Then the query result should have 1 document
    And the result should contain a document with the field "action" having the value "LOGIN_SUCCESS"

  Scenario: Filter records using a timestamp range
    Given a running TissDB instance
    And a collection named "metrics" exists
    When I execute the TissQL query "INSERT INTO metrics (value, event_time) VALUES (100, '2025-03-20T12:00:00Z')" on "metrics"
    And I execute the TissQL query "INSERT INTO metrics (value, event_time) VALUES (105, '2025-03-20T12:05:00Z')" on "metrics"
    And I execute the TissQL query "INSERT INTO metrics (value, event_time) VALUES (110, '2025-03-20T12:10:00Z')" on "metrics"
    And I execute the TissQL query "INSERT INTO metrics (value, event_time) VALUES (115, '2025-03-20T12:15:00Z')" on "metrics"
    When I execute the TissQL query "SELECT value FROM metrics WHERE event_time > '2025-03-20T12:01:00Z' AND event_time < '2025-03-20T12:12:00Z'" on "metrics"
    Then the query result should have 2 documents
    And the result should contain a document with the field "value" having the value 105
    And the result should contain a document with the field "value" having the value 110

  Scenario: Insert and select a Timestamp value with millisecond precision
    Given a running TissDB instance
    And a collection named "traces" exists
    When I execute the TissQL query "INSERT INTO traces (span_id, event_time) VALUES ('span-1', '2025-04-01T14:30:15.123Z')" on "traces"
    Then the query should succeed
    When I execute the TissQL query "SELECT span_id, event_time FROM traces WHERE span_id = 'span-1'" on "traces"
    Then the query result should have 1 document
    And the document should contain the field "event_time" with the timestamp "2025-04-01T14:30:15.123Z"
