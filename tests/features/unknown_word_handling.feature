Feature: Unknown Word Handling
  As a user, I want the model to handle words that are not in its vocabulary.

  Scenario: Tokenize a sentence with an unknown word
    Given a QuantaTissu model with a fixed vocabulary
    And a sentence "this is a foobar test"
    When I tokenize the sentence
    Then the token "foobar" should be mapped to the "<unk>" token ID
