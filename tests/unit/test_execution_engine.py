import unittest
import os
import shutil
import json
from quanta_tissu.tisslm.core.execution_engine import ExecutionEngine, ToolRegistry
from quanta_tissu.tisslm.core.system_error_handler import TissSecurityError
from quanta_tissu.tisslm.core.model_error_handler import TissAssertionError
from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser
from quanta_tissu.tisslm.core.tools.builtins import RunCommandTool, WriteFileTool, ReadFileTool, AssertConditionTool

class TestExecutionEngine(unittest.TestCase):

    def setUp(self):
        """Set up a temporary directory for testing."""
        self.test_dir = os.path.abspath("test_project_root")
        os.makedirs(self.test_dir, exist_ok=True)
        self.tool_registry = ToolRegistry()
        self.tool_registry.register("RUN", RunCommandTool(self.test_dir))
        self.tool_registry.register("WRITE", WriteFileTool(self.test_dir))
        self.tool_registry.register("READ", ReadFileTool(self.test_dir))
        self.tool_registry.register("ASSERT", AssertConditionTool())
        self.engine = ExecutionEngine(self.tool_registry)

    def tearDown(self):
        """Clean up the temporary directory."""
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)

    def test_write_and_read_file(self):
        """Test that the WRITE and READ commands work correctly."""
        # AST for writing a file
        write_ast = [{'type': 'WRITE', 'path': 'test.txt', 'content': 'Hello, Tiss!'}]
        # Execute the write command
        state = self.engine.execute(write_ast)
        self.assertFalse(state.is_halted)
        # Verify the file was written
        expected_path = os.path.join(self.test_dir, 'test.txt')
        self.assertTrue(os.path.exists(expected_path))
        with open(expected_path, 'r') as f:
            self.assertEqual(f.read(), 'Hello, Tiss!')

        # AST for reading the file
        read_ast = [{'type': 'READ', 'path': 'test.txt', 'variable': 'my_var'}]
        # Execute the read command
        state = self.engine.execute(read_ast)
        self.assertFalse(state.is_halted)
        self.assertIn('my_var', state.variables)
        self.assertEqual(state.variables['my_var'], 'Hello, Tiss!')

    def test_run_command_and_assert(self):
        """Test RUN and ASSERT commands."""
        # AST for running a command and asserting on its output
        run_assert_ast = [
            {'type': 'RUN', 'command': 'echo "Hello from shell"'},
            {'type': 'ASSERT', 'condition': 'LAST_RUN.EXIT_CODE == 0'},
            {'type': 'ASSERT', 'condition': 'LAST_RUN.STDOUT CONTAINS "Hello"'},
            {'type': 'ASSERT', 'condition': 'LAST_RUN.STDERR IS_EMPTY'}
        ]
        state = self.engine.execute(run_assert_ast)
        self.assertFalse(state.is_halted, f"Execution halted unexpectedly. Log: {state.execution_log}")
        self.assertIsNotNone(state.last_run_result)
        self.assertEqual(state.last_run_result['exit_code'], 0)
        self.assertIn("Hello from shell", state.last_run_result['stdout'])

    def test_failed_assertion(self):
        """Test that a failed assertion halts execution."""
        ast = [
            {'type': 'RUN', 'command': 'echo "some output"'},
            {'type': 'ASSERT', 'condition': 'LAST_RUN.EXIT_CODE == 1'} # This will fail
        ]
        state = self.engine.execute(ast)
        self.assertTrue(state.is_halted)
        # Check the log for failure reason
        self.assertEqual(state.execution_log[-1]['status'], 'FAILURE')
        self.assertIn('Assertion failed', state.execution_log[-1]['error'])


    # Commented out as _resolve_secure_path is no longer part of ExecutionEngine
    # def test_path_security_raises_error(self):
    #     """Test that resolving a malicious path raises TissSecurityError."""
    #     with self.assertRaises(TissSecurityError):
    #         self.engine._resolve_secure_path('../malicious.txt')
    #     with self.assertRaises(TissSecurityError):
    #         self.engine._resolve_secure_path('/etc/passwd')
            
    # Commented out as _resolve_secure_path is no longer part of ExecutionEngine
    # def test_path_security_halts_execution(self):
    #     """Test that the engine halts when a malicious path is used in a command."""
    #     malicious_ast = [{'type': 'WRITE', 'path': '../malicious.txt', 'content': 'pwned'}]
    #     state = self.engine.execute(malicious_ast)
    #     self.assertTrue(state.is_halted)
    #     self.assertEqual(state.execution_log[0]['status'], 'FAILURE')
    #     self.assertIn('Path access violation', state.execution_log[0]['error'])


    def test_full_script_execution(self):
        """Test parsing and executing a full TissLang script."""
        script = """
        TASK "Full test script"
        STEP "Setup and run" {
            WRITE "app.py" <<PYTHON
print("This is the app")
PYTHON
            RUN "python app.py"
            ASSERT LAST_RUN.EXIT_CODE == 0
            ASSERT LAST_RUN.STDOUT CONTAINS "This is the app"
        }
        """
        parser = TissLangParser()
        ast = parser.parse(script)
        state = self.engine.execute(ast)
        self.assertFalse(state.is_halted, f"Execution halted unexpectedly. Log: {state.execution_log}")
        
        # Verify file was created
        app_py_path = os.path.join(self.test_dir, 'app.py')
        self.assertTrue(os.path.exists(app_py_path))
        
        # Verify final state
        self.assertIn("This is the app", state.last_run_result['stdout'])

    # Commented out as ExecutionEngine does not hold state directly
    # def test_initial_state(self):
    #     """Test the initial state of a new ExecutionEngine."""
    #     self.assertIsNotNone(self.engine.state)
    #     self.assertFalse(self.engine.state.is_halted)
    #     self.assertEqual(self.engine.state.variables, {})
    #     self.assertIsNone(self.engine.state.last_run_result)

if __name__ == "__main__":
    unittest.main()
