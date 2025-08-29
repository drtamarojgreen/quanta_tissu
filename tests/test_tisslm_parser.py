import sys
import os
import unittest

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.parser.parser import TissLangParser, TissLangParserError

class TestTissLangParser(unittest.TestCase):

    def test_parse_simple_step(self):
        """
        Tests parsing a simple, valid TissLang script with one STEP and one RUN command.
        """
        script = """
        STEP "A simple step" {
          RUN "echo \"Hello\""
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1, "AST should have one top-level node for the STEP.")

        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(step_node['description'], 'A simple step')
        self.assertIn('commands', step_node)

        commands = step_node['commands']
        self.assertEqual(len(commands), 1, "STEP should contain one command.")

        run_node = commands[0]
        self.assertEqual(run_node['type'], 'RUN')
        self.assertEqual(run_node['command'], 'echo "Hello"')

    def test_parse_invalid_command(self):
        """
        Tests that the parser raises an error for an invalid command.
        """
        script = "INVALID COMMAND"
        parser = TissLangParser()

        with self.assertRaises(TissLangParserError, msg="Parser should raise TissLangParserError for invalid syntax."):
            parser.parse(script)

if __name__ == '__main__':
    unittest.main()

    def test_parse_log_command(self):
        """
        Tests parsing a LOG command within a STEP.
        """
        script = """
        STEP "Log test" {
          LOG "This is a log message"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 1)

        log_node = step_node['commands'][0]
        self.assertEqual(log_node['type'], 'LOG')
        self.assertEqual(log_node['message'], 'This is a log message')

    def test_parse_setup_command(self):
        """
        Tests parsing a SETUP command with a description.
        """
        script = """
        SETUP "Initial setup for project" {
          RUN "npm install"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        setup_node = ast[0]
        self.assertEqual(setup_node['type'], 'SETUP')
        self.assertEqual(setup_node['description'], 'Initial setup for project')
        self.assertIn('commands', setup_node)

        commands = setup_node['commands']
        self.assertEqual(len(commands), 1)
        run_node = commands[0]
        self.assertEqual(run_node['type'], 'RUN')
        self.assertEqual(run_node['command'], 'npm install')

    def test_parse_parallel_step(self):
        """
        Tests parsing a PARALLEL step.
        """
        script = """
        PARALLEL {
          RUN "command1"
          RUN "command2"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        parallel_node = ast[0]
        self.assertEqual(parallel_node['type'], 'PARALLEL')
        self.assertIn('commands', parallel_node)
        self.assertEqual(len(parallel_node['commands']), 2)

    def test_parse_choose_step(self):
        """
        Tests parsing a CHOOSE step.
        """
        script = """
        CHOOSE {
          STEP "Option A" { RUN "option_a_cmd" }
          STEP "Option B" { RUN "option_b_cmd" }
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        choose_node = ast[0]
        self.assertEqual(choose_node['type'], 'CHOOSE')
        self.assertIn('commands', choose_node)
        self.assertEqual(len(choose_node['commands']), 2)
        self.assertEqual(choose_node['commands'][0]['type'], 'STEP')
        self.assertEqual(choose_node['commands'][1]['type'], 'STEP')

    def test_parse_estimate_cost_step(self):
        """
        Tests parsing an ESTIMATE_COST step.
        """
        script = """
        ESTIMATE_COST {
          PROMPT_AGENT "Estimate cost of this task"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        estimate_node = ast[0]
        self.assertEqual(estimate_node['type'], 'ESTIMATE_COST')
        self.assertIn('commands', estimate_node)
        self.assertEqual(len(estimate_node['commands']), 1)
        self.assertEqual(estimate_node['commands'][0]['type'], 'PROMPT_AGENT')

    def test_parse_set_budget_command_string(self):
        """
        Tests parsing a SET_BUDGET command with a string value.
        """
        script = """
        STEP "Budget test" {
          SET_BUDGET EXECUTION_TIME = "5m"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 1)

        budget_node = step_node['commands'][0]
        self.assertEqual(budget_node['type'], 'SET_BUDGET')
        self.assertEqual(budget_node['variable'], 'EXECUTION_TIME')
        self.assertEqual(budget_node['value'], '5m')

    def test_parse_set_budget_command_number(self):
        """
        Tests parsing a SET_BUDGET command with a number value.
        """
        script = """
        STEP "Budget test" {
          SET_BUDGET API_CALLS = 100
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 1)

        budget_node = step_node['commands'][0]
        self.assertEqual(budget_node['type'], 'SET_BUDGET')
        self.assertEqual(budget_node['variable'], 'API_CALLS')
        self.assertEqual(budget_node['value'], 100)

    def test_parse_set_budget_command_boolean(self):
        """
        Tests parsing a SET_BUDGET command with a boolean value.
        """
        script = """
        STEP "Budget test" {
          SET_BUDGET ENABLE_LOGGING = true
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 1)

        budget_node = step_node['commands'][0]
        self.assertEqual(budget_node['type'], 'SET_BUDGET')
        self.assertEqual(budget_node['variable'], 'ENABLE_LOGGING')
        self.assertEqual(budget_node['value'], True)

    def test_parse_set_budget_command_object(self):
        """
        Tests parsing a SET_BUDGET command with an object value.
        """
        script = """
        STEP "Budget test" {
          SET_BUDGET CONFIG = {"key": "value", "num": 123}
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 1)

        budget_node = step_node['commands'][0]
        self.assertEqual(budget_node['type'], 'SET_BUDGET')
        self.assertEqual(budget_node['variable'], 'CONFIG')
        self.assertEqual(budget_node['value'], {"key": "value", "num": 123})

    def test_parse_set_budget_command_list(self):
        """
        Tests parsing a SET_BUDGET command with a list value.
        """
        script = """
        STEP "Budget test" {
          SET_BUDGET ITEMS = ["item1", 2, true]
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 1)

        budget_node = step_node['commands'][0]
        self.assertEqual(budget_node['type'], 'SET_BUDGET')
        self.assertEqual(budget_node['variable'], 'ITEMS')
        self.assertEqual(budget_node['value'], ["item1", 2, True])

    def test_parse_request_review_command(self):
        """
        Tests parsing a REQUEST_REVIEW command.
        """
        script = """
        STEP "Review test" {
          REQUEST_REVIEW "Please review this code change."
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 1)

        review_node = step_node['commands'][0]
        self.assertEqual(review_node['type'], 'REQUEST_REVIEW')
        self.assertEqual(review_node['message'], 'Please review this code change.')

    def test_parse_if_then_boolean(self):
        """
        Tests parsing an IF THEN statement with a boolean condition.
        """
        script = """
        IF true THEN {
          LOG "Condition is true"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        if_node = ast[0]
        self.assertEqual(if_node['type'], 'IF')
        self.assertEqual(if_node['condition']['type'], 'BOOLEAN')
        self.assertTrue(if_node['condition']['value'])
        self.assertIsNotNone(if_node['then_block'])
        self.assertEqual(len(if_node['then_block']), 1)
        self.assertEqual(if_node['then_block'][0]['type'], 'LOG')
        self.assertIsNone(if_node['else_block'])

    def test_parse_if_then_else_comparison(self):
        """
        Tests parsing an IF THEN ELSE statement with a comparison condition.
        """
        script = """
        IF var == "value" THEN {
          LOG "Var is value"
        } ELSE {
          LOG "Var is not value"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        if_node = ast[0]
        self.assertEqual(if_node['type'], 'IF')
        self.assertEqual(if_node['condition']['type'], 'COMPARISON')
        self.assertEqual(if_node['condition']['left'], 'var')
        self.assertEqual(if_node['condition']['operator'], '==')
        self.assertEqual(if_node['condition']['right'], 'value')
        self.assertIsNotNone(if_node['then_block'])
        self.assertEqual(len(if_node['then_block']), 1)
        self.assertEqual(if_node['then_block'][0]['type'], 'LOG')
        self.assertIsNotNone(if_node['else_block'])
        self.assertEqual(len(if_node['else_block']), 1)
        self.assertEqual(if_node['else_block'][0]['type'], 'LOG')

    def test_parse_if_nested(self):
        """
        Tests parsing nested IF statements.
        """
        script = """
        IF true THEN {
          IF false THEN {
            LOG "Inner true"
          } ELSE {
            LOG "Inner false"
          }
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        outer_if = ast[0]
        self.assertEqual(outer_if['type'], 'IF')
        self.assertEqual(len(outer_if['then_block']), 1)

        inner_if = outer_if['then_block'][0]
        self.assertEqual(inner_if['type'], 'IF')
        self.assertEqual(len(inner_if['then_block']), 1)
        self.assertEqual(len(inner_if['else_block']), 1)

    def test_parse_program_level_directive(self):
        """
        Tests parsing a program-level directive.
        """
        script = """
        @persona "senior_developer"
        TASK "Test task" {}
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertIn('metadata', parser.__dict__) # Check if metadata exists
        self.assertIn('directives', parser.metadata)
        self.assertEqual(len(parser.metadata['directives']), 1)
        directive = parser.metadata['directives'][0]
        self.assertEqual(directive['name'], 'persona')
        self.assertEqual(directive['value'], 'senior_developer')

        self.assertEqual(len(ast), 1)
        self.assertEqual(ast[0]['type'], 'TASK')

    def test_parse_in_step_directive(self):
        """
        Tests parsing a directive within a STEP block.
        """
        script = """
        STEP "Test step with directive" {
          @output "json"
          PROMPT_AGENT "Generate JSON"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        step_node = ast[0]
        self.assertEqual(step_node['type'], 'STEP')
        self.assertEqual(len(step_node['commands']), 2) # Directive + PROMPT_AGENT

        directive_node = step_node['commands'][0]
        self.assertEqual(directive_node['type'], 'DIRECTIVE')
        self.assertEqual(directive_node['name'], 'output')
        self.assertEqual(directive_node['value'], 'json')

        prompt_node = step_node['commands'][1]
        self.assertEqual(prompt_node['type'], 'PROMPT_AGENT')

    def test_parse_task_with_block(self):
        """
        Tests parsing a TASK with an associated block.
        """
        script = """
        TASK "My Task" {
          STEP "First step" {
            LOG "Starting task"
          }
          STEP "Second step" {
            RUN "do_something"
          }
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)

        self.assertEqual(len(ast), 1)
        task_node = ast[0]
        self.assertEqual(task_node['type'], 'TASK')
        self.assertEqual(task_node['description'], 'My Task')
        self.assertIn('commands', task_node)
        self.assertEqual(len(task_node['commands']), 2)

        step1 = task_node['commands'][0]
        self.assertEqual(step1['type'], 'STEP')
        self.assertEqual(step1['description'], 'First step')
        self.assertEqual(len(step1['commands']), 1)
        self.assertEqual(step1['commands'][0]['type'], 'LOG')

        step2 = task_node['commands'][1]
        self.assertEqual(step2['type'], 'STEP')
        self.assertEqual(step2['description'], 'Second step')
        self.assertEqual(len(step2['commands']), 1)
        self.assertEqual(step2['commands'][0]['type'], 'RUN')

