Feature: TissLang C++ IDE Functionality

  Scenario: Opening a Tiss file in the IDE
    Given the TissLang IDE is running
    When I open the file "example.tiss"
    Then the content of "example.tiss" should be displayed in the editor
    And the syntax for TissLang should be highlighted

  Scenario: Basic text editing
    Given a new, empty file is open in the TissLang IDE
    When I type the text "SETUP { db = 'test.db' }"
    Then the editor should contain the text "SETUP { db = 'test.db' }"
    And the keyword "SETUP" should be highlighted

  Scenario: Syntax highlighting for comments and strings
    Given a file is open in the TissLang IDE
    When the editor contains the text "# This is a comment\nACTION { query = 'SELECT * FROM users' }"
    Then the line "# This is a comment" should be highlighted as a comment
    And the text "'SELECT * FROM users'" should be highlighted as a string

  Scenario: Using the search functionality
    Given a file is open in the TissLang IDE with the content "first line\nsecond line\nthird line"
    When I search for the text "second line"
    Then the text "second line" should be selected in the editor

  Scenario: Saving a file
    Given a file is open in the TissLang IDE with unsaved changes
    When I save the file as "new_file.tiss"
    Then a file named "new_file.tiss" should be created with the editor's content
