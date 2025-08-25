import unittest
import sys
import os

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser, TissLangParserError

class TestTissLangParser(unittest.TestCase):

    def test_parse_task(self):
        parser = TissLangParser()
        script = 'TASK "This is a test task"'
        expected_ast = [{'type': 'TASK', 'description': 'This is a test task'}]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_step(self):
        parser = TissLangParser()
        script = '''
        STEP "This is a test step" {
            RUN "echo 'hello'"
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'This is a test step',
            'commands': [{'type': 'RUN', 'command': "echo 'hello'"}]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_run(self):
        parser = TissLangParser()
        script = '''
        STEP "Run a command" {
            RUN "ls -l"
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Run a command',
            'commands': [{'type': 'RUN', 'command': 'ls -l'}]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_write(self):
        parser = TissLangParser()
        script = '''
        STEP "Write a file" {
            WRITE "test.txt" <<EOF
            Hello, world!
            EOF
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Write a file',
            'commands': [{'type': 'WRITE', 'path': 'test.txt', 'language': 'EOF', 'content': 'Hello, world!'}]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_read(self):
        parser = TissLangParser()
        script = '''
        STEP "Read a file" {
            READ "test.txt" AS my_var
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Read a file',
            'commands': [{'type': 'READ', 'path': 'test.txt', 'variable': 'my_var'}]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_assert(self):
        parser = TissLangParser()
        script = '''
        STEP "Assert a condition" {
            ASSERT LAST_RUN.EXIT_CODE == 0
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Assert a condition',
            'commands': [{'type': 'ASSERT', 'condition': 'LAST_RUN.EXIT_CODE == 0'}]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_setup(self):
        parser = TissLangParser()
        script = '''
        SETUP {
            RUN "pip install -r requirements.txt"
        }
        '''
        expected_ast = [{
            'type': 'SETUP',
            'commands': [{'type': 'RUN', 'command': 'pip install -r requirements.txt'}]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_unclosed_block(self):
        parser = TissLangParser()
        script = '''
        STEP "This step is not closed" {
        '''
        with self.assertRaises(TissLangParserError):
            parser.parse(script)

    def test_parse_run_with_single_quotes_and_inner_quotes(self):
        parser = TissLangParser()
        script = '''
        STEP "Run a command with quotes" {
            RUN 'echo "Hello, quoted world!"'
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Run a command with quotes',
            'commands': [{'type': 'RUN', 'command': 'echo "Hello, quoted world!"'}]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)


    def test_parse_simple_if(self):
        parser = TissLangParser()
        script = '''
        STEP "Test IF" {
            IF "VAR == 1"
                RUN "echo 'true'"
            ENDIF
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Test IF',
            'commands': [{
                'type': 'IF',
                'condition': 'VAR == 1',
                'true_branch': [{'type': 'RUN', 'command': "echo 'true'"}],
                'false_branch': []
            }]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_if_else(self):
        parser = TissLangParser()
        script = '''
        STEP "Test IF/ELSE" {
            IF "VAR == 1"
                RUN "echo 'true'"
            ELSE
                RUN "echo 'false'"
            ENDIF
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Test IF/ELSE',
            'commands': [{
                'type': 'IF',
                'condition': 'VAR == 1',
                'true_branch': [{'type': 'RUN', 'command': "echo 'true'"}],
                'false_branch': [{'type': 'RUN', 'command': "echo 'false'"}]
            }]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_parse_nested_if(self):
        parser = TissLangParser()
        script = '''
        STEP "Test Nested IF" {
            IF "VAR > 10"
                IF "VAR < 20"
                    RUN "echo 'in range'"
                ENDIF
            ENDIF
        }
        '''
        expected_ast = [{
            'type': 'STEP',
            'description': 'Test Nested IF',
            'commands': [{
                'type': 'IF',
                'condition': 'VAR > 10',
                'true_branch': [{
                    'type': 'IF',
                    'condition': 'VAR < 20',
                    'true_branch': [{'type': 'RUN', 'command': "echo 'in range'"}],
                    'false_branch': []
                }],
                'false_branch': []
            }]
        }]
        ast = parser.parse(script)
        self.assertEqual(ast, expected_ast)

    def test_error_else_without_if(self):
        parser = TissLangParser()
        script = '''
        STEP "Error" {
            ELSE
        }
        '''
        with self.assertRaisesRegex(TissLangParserError, "ELSE found without a preceding IF."):
            parser.parse(script)

    def test_error_endif_without_if(self):
        parser = TissLangParser()
        script = '''
        STEP "Error" {
            ENDIF
        }
        '''
        with self.assertRaisesRegex(TissLangParserError, "ENDIF found without a preceding IF."):
            parser.parse(script)

    def test_error_unclosed_if(self):
        parser = TissLangParser()
        script = '''
        STEP "Unclosed IF" {
            IF "VAR == 1"
        '''
        with self.assertRaisesRegex(TissLangParserError, "Unexpected end of script. Current state is 'IN_IF'. A block may be unclosed."):
            parser.parse(script)

if __name__ == '__main__':
    unittest.main()
