import sys
import os
import json

# Add project root to the Python path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser, TissLangParserError
from quanta_tissu.tisslm.core.execution_engine import ExecutionEngine, ToolRegistry
from quanta_tissu.tisslm.core.system_error_handler import FileIOError, TissSystemError, TissCommandError
from quanta_tissu.tisslm.core.builtin_tools import run_command, write_file, read_file, assert_condition
from quanta_tissu.tisslm.core.tools.base import BaseTool

# --- Tool Classes ---
# The ExecutionEngine expects tool objects with an `execute` method.
# We create wrapper classes for the existing tool functions.

class RunCommandTool(BaseTool):
    def execute(self, state, args):
        run_command(state, args)

class WriteFileTool(BaseTool):
    def execute(self, state, args):
        write_file(state, args)

class ReadFileTool(BaseTool):
    def execute(self, state, args):
        read_file(state, args)

class AssertConditionTool(BaseTool):
    def execute(self, state, args):
        assert_condition(state, args)

class PromptAgentTool(BaseTool):
    """A mock tool for PROMPT_AGENT."""
    def execute(self, state, args):
        prompt = args.get("prompt")
        variable_name = args.get("variable")
        print(f"--- MOCK PROMPT_AGENT: Received prompt: '{prompt}' ---")

        # This is a mock response. In a real scenario, this would call the LLM.
        fib_code = '''def fibonacci(n):
    """
    Calculates the nth Fibonacci number.
    """
    if n <= 0:
        return 0
    elif n == 1:
        return 1
    else:
        a, b = 0, 1
        for _ in range(2, n + 1):
            a, b = b, a + b
        return b'''

        if variable_name:
            state.variables[variable_name] = fib_code.strip()
        else:
            # If no variable is specified, we don't store the result.
            # This could be changed to store in a default variable if needed.
            pass

def main():
    """
    This script demonstrates code generation using a TissLang script
    executed from Python.
    """
    # 1. Define the TissLang script in a multiline string
    tiss_script = """
    TASK "Generate a Fibonacci function using the agent"

    STEP "Generate Python code for Fibonacci sequence" {
        PROMPT_AGENT "Generate a Python function to calculate the nth Fibonacci number." INTO fib_code
    }

    STEP "Write the generated code to a file" {
        WRITE "fibonacci.py" <<PYTHON
{{fib_code}}
PYTHON
    }

    STEP "Verify the file was created" {
        READ "fibonacci.py" AS file_content
        ASSERT LAST_RUN.FILE_CONTENT CONTAINS "def fibonacci(n):"
    }
    """

    # 2. Instantiate TissLang components
    parser = TissLangParser()
    registry = ToolRegistry()
    engine = ExecutionEngine(registry)

    # 3. Register tool instances
    registry.register("RUN", RunCommandTool())
    registry.register("WRITE", WriteFileTool())
    registry.register("READ", ReadFileTool())
    registry.register("ASSERT", AssertConditionTool())
    registry.register("PROMPT_AGENT", PromptAgentTool())

    print("--- Starting TissLang script for Code Generation ---")

    try:
        # 4. Parse the script to get the AST
        ast = parser.parse(tiss_script)
        print("--- Script Parsed Successfully ---")

        # 5. Execute the AST
        final_state = engine.execute(ast)
        print("\n--- Execution Complete ---")

        if final_state.is_halted:
            print("--- Execution Halted with an Error ---")
            sys.exit(1)
        else:
            print("--- Script Executed Successfully ---")
            print("Generated file 'fibonacci.py' should now exist.")

    except (TissLangParserError, TissCommandError, FileIOError, TissSystemError) as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
