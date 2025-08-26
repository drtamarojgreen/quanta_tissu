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
    """A mock tool for PROMPT_AGENT that 'refactors' code."""
    def execute(self, state, args):
        prompt = args.get("prompt")
        variable_name = args.get("variable")
        print(f"--- MOCK PROMPT_AGENT: Received prompt: '{prompt}' ---")

        # This is a mock response for refactoring.
        refactored_code = '''
def calculate_sum(a, b):
    """
    Calculates the sum of two numbers.
    """
    return a + b
'''

        if variable_name:
            state.variables[variable_name] = refactored_code.strip()

def main():
    """
    This script demonstrates code refactoring using a TissLang script
    executed from Python.
    """
    # 1. Create the initial file with poorly formatted code
    bad_code = """
def calculate_sum( a, b):
  #This function adds two numbers
  return a+b
"""
    try:
        with open("bad_code.py", "w") as f:
            f.write(bad_code)
        print("--- Created initial 'bad_code.py' file ---")
    except IOError as e:
        print(f"Error creating initial file: {e}")
        sys.exit(1)

    # 2. Define the TissLang script for refactoring
    tiss_script = """
    TASK "Refactor a Python script"

    STEP "Read the poorly formatted code" {
        READ "bad_code.py" AS original_code
    }

    STEP "Use the agent to refactor the code" {
        PROMPT_AGENT "Refactor this Python code for clarity and PEP8 compliance: {{original_code}}" INTO refactored_code
    }

    STEP "Write the refactored code to a new file" {
        WRITE "good_code.py" <<PYTHON
{{refactored_code}}
PYTHON
    }

    STEP "Verify the new file was created and contains refactored code" {
        READ "good_code.py" AS final_content
        ASSERT LAST_RUN.FILE_CONTENT CONTAINS "def calculate_sum(a, b):"
    }
    """

    # 3. Instantiate TissLang components
    parser = TissLangParser()
    registry = ToolRegistry()
    engine = ExecutionEngine(registry)

    # 4. Register tool instances
    registry.register("RUN", RunCommandTool())
    registry.register("WRITE", WriteFileTool())
    registry.register("READ", ReadFileTool())
    registry.register("ASSERT", AssertConditionTool())
    registry.register("PROMPT_AGENT", PromptAgentTool())

    print("--- Starting TissLang script for Code Refactoring ---")

    try:
        # 5. Parse and execute the script
        ast = parser.parse(tiss_script)
        print("--- Script Parsed Successfully ---")
        final_state = engine.execute(ast)
        print("\n--- Execution Complete ---")

        if final_state.is_halted:
            print("--- Execution Halted with an Error ---")
            sys.exit(1)
        else:
            print("--- Script Executed Successfully ---")
            print("Generated file 'good_code.py' should now exist.")

    except (TissLangParserError, TissCommandError, FileIOError, TissSystemError) as e:
        print(f"An error occurred: {e}")
        sys.exit(1)
    finally:
        # Clean up the created files
        if os.path.exists("bad_code.py"):
            os.remove("bad_code.py")
        if os.path.exists("good_code.py"):
            os.remove("good_code.py")
        print("--- Cleaned up generated files ---")

if __name__ == "__main__":
    main()
