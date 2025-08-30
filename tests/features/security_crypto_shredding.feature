Feature: Security - Cryptographic Shredding

  Background:
    Given a running TissDB instance
    And a user with a valid "admin" token

  Scenario: Deleting a collection makes its old data unrecoverable
    Given a collection named "shred_me" exists
    And I create a document with ID "temp_doc" and content '{"data": "very sensitive data"}' in "shred_me"
    And I force a flush of the memtable for "shred_me"
    When I delete the collection "shred_me"
    And a collection named "shred_me" exists
    Then attempting to read the document with ID "temp_doc" from "shred_me" should fail
