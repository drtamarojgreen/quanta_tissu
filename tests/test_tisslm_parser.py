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
