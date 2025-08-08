Feature: TissLang Parser

  Scenario: Parsing a simple TissLang script
    Given a TissLang script:
      """
      TASK "A simple task"
      """
    When I parse the script
    Then the AST should be:
      """
      [
        {
          "type": "TASK",
          "description": "A simple task"
        }
      ]
      """

  Scenario: Parsing a STEP with a RUN command
    Given a TissLang script:
      """
      STEP "Run a command" {
        RUN "echo 'Hello, BDD!'"
      }
      """
    When I parse the script
    Then the AST should be:
      """
      [
        {
          "type": "STEP",
          "description": "Run a command",
          "commands": [
            {
              "type": "RUN",
              "command": "echo 'Hello, BDD!'"
            }
          ]
        }
      ]
      """

  Scenario: Parsing a WRITE command
    Given a TissLang script:
      """
      STEP "Write to a file" {
        WRITE "greetings.txt" <<EOF
      Hello from TissLang!
      This is a test.
      EOF
      }
      """
    When I parse the script
    Then the AST should be:
      """
      [
        {
          "type": "STEP",
          "description": "Write to a file",
          "commands": [
            {
              "type": "WRITE",
              "path": "greetings.txt",
              "language": "EOF",
              "content": "Hello from TissLang!\nThis is a test."
            }
          ]
        }
      ]
      """

  Scenario: Parsing a script with multiple commands
    Given a TissLang script:
      """
      TASK "A complex task with multiple steps"

      SETUP {
        RUN "mkdir -p temp"
      }

      STEP "First step" {
        READ "input.txt" AS input_data
        RUN "process_data.py --input={{input_data}}"
      }

      STEP "Second step" {
        ASSERT LAST_RUN.EXIT_CODE == 0
      }
      """
    When I parse the script
    Then the AST should have 4 top-level nodes

  Scenario: Parsing a script with a syntax error
    Given a TissLang script:
      """
      STEP "This will fail" {
        RUN "some command"
      # Missing closing brace
      """
    When I parse the script
    Then parsing should fail with an error containing "Unexpected end of script"
