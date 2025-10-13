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

class LogTool(BaseTool):
    """A simple tool to log messages."""
    def execute(self, state, args):
        message = args.get("message")
        print(f"[AGENT LOG]: {message}")

class PromptAgentTool(BaseTool):
    """A mock tool for PROMPT_AGENT that 'summarizes' text."""
    def execute(self, state, args):
        prompt = args.get("prompt")
        variable_name = args.get("variable")
        print(f"--- MOCK PROMPT_AGENT: Received prompt: '{prompt}' ---")

        summary = "This is a concise summary of the provided text."

        if variable_name:
            state.variables[variable_name] = summary

def main():
    """
    This script demonstrates text summarization using a TissLang script
    executed from Python.
    """
    # 1. Create the initial file with a long paragraph
    long_text = """
Artificial intelligence (AI) is a wide-ranging branch of computer science concerned with building smart machines capable of performing tasks that typically require human intelligence. AI is an interdisciplinary science with multiple approaches, but advancements in machine learning and deep learning are creating a paradigm shift in virtually every sector of the tech industry. The long-term goal of AI research is to create general AI, which would be able to perform any intellectual task that a human can.
"""
    try:
        with open("long_text.txt", "w") as f:
            f.write(long_text)
        print("--- Created initial 'long_text.txt' file ---")
    except IOError as e:
        print(f"Error creating initial file: {e}")
        sys.exit(1)

    # 2. Define the TissLang script for summarization
    tiss_script = """
    TASK "Summarize a text document"

    STEP "Read the text file" {
        READ "long_text.txt" AS document
    }

    STEP "Use the agent to summarize the text" {
        PROMPT_AGENT "Please summarize this document: {{document}}" INTO summary
    }

    STEP "Log the summary to the console" {
        LOG "Summary from agent: {{summary}}"
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
    registry.register("LOG", LogTool())
    registry.register("PROMPT_AGENT", PromptAgentTool())

    print("--- Starting TissLang script for Text Summarization ---")

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

    except (TissLangParserError, TissCommandError, FileIOError, TissSystemError) as e:
        print(f"An error occurred: {e}")
        sys.exit(1)
    finally:
        # Clean up the created file
        if os.path.exists("long_text.txt"):
            os.remove("long_text.txt")
        print("--- Cleaned up generated file ---")

if __name__ == "__main__":
    main()
