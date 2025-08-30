Feature: Security - Encryption at Rest

  Background:
    Given a running TissDB instance
    And a user with a valid "admin" token
    And a collection named "encrypted_collection" exists

  Scenario: Stored document values are not in plaintext
    Given I create a document with ID "secret_doc" and content '{"secret_message": "the eagle flies at midnight"}' in "encrypted_collection"
    When I inspect the raw data files for "encrypted_collection"
    Then the plaintext "the eagle flies at midnight" should not be found
