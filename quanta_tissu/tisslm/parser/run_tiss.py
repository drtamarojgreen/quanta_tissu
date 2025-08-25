import argparse
import json
from typing import List, Dict, Any

# Adjusting imports to be relative to the project root
# This assumes the script is run from the root of the project.
import sys
import os
sys.path.append(os.getcwd())

from ..integrations.tisslang_parser import TissLangParser, TissLangParserError
from ..core.execution_engine import ExecutionEngine, ToolRegistry
from ..core.tools import run_command, write_file, read_file, assert_condition, TissCommandError

def main():
    """
    Main entry point for the TissLang script runner.
    Parses, and executes a .tiss file.
    """
    parser = argparse.ArgumentParser(description="Execute a TissLang (.tiss) script.")
    parser.add_argument("filepath", help="The path to the .tiss script file.")
    args = parser.parse_args()

    try:
        with open(args.filepath, "r", encoding="utf-8") as f:
            script_content = f.read()
    except FileNotFoundError:
        print(f"Error: The file '{args.filepath}' was not found.")
        sys.exit(1)
    except IOError as e:
        print(f"Error reading file '{args.filepath}': {e}")
        sys.exit(1)

    # 1. Instantiate components
    parser = TissLangParser()
    registry = ToolRegistry()
    engine = ExecutionEngine(registry)

    # 2. Register all available tools
    registry.register("RUN", run_command)
    registry.register("WRITE", write_file)
    registry.register("READ", read_file)
    registry.register("ASSERT", assert_condition)

    # 3. Parse the script to get the AST
    try:
        ast = parser.parse(script_content)
        print("--- Script Parsed Successfully ---")
        # print(json.dumps(ast, indent=2)) # Optional: for debugging the AST
    except TissLangParserError as e:
        print(f"Failed to parse TissLang script: {e}")
        sys.exit(1)

    # 4. Execute the AST
    try:
        final_state = engine.execute(ast)
        # Optional: Print final state for debugging
        # print("\n--- Final Execution State ---")
        # print(f"Halted: {final_state.is_halted}")
        # print(f"Variables: {json.dumps(final_state.variables, indent=2)}")
        # print(f"Last Run Result: {final_state.last_run_result}")
        
        if final_state.is_halted:
            sys.exit(1) # Exit with an error code if execution was halted

    except (TissCommandError, KeyError) as e:
        print(f"An error occurred during execution: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
