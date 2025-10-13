import subprocess
import sys
import os
import re
import ast
import inspect

# Add the project root to the Python path to allow for package imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

def run_tests():
    """
    Runs the main test suite and captures the output.
    """
    print("--- Running test suite... ---")
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
    command = [sys.executable, "run_tests.py"]
    result = subprocess.run(command, cwd=root_dir, capture_output=True, text=True)
    print(result.stdout)
    if result.stderr:
        print("--- Stderr ---")
        print(result.stderr)
    return result

def extract_function_code(file_path, function_name):
    """
    Extracts the source code of a specific function from a file using AST.
    """
    try:
        with open(file_path, 'r') as f:
            file_content = f.read()
        tree = ast.parse(file_content)
        for node in ast.walk(tree):
            if isinstance(node, ast.FunctionDef) and node.name == function_name:
                return ast.get_source_segment(file_content, node)
    except Exception as e:
        print(f"Error extracting function code: {e}")
    return None

def parse_failure(test_output):
    """
    Parses test output to find failing file, function, and error details.
    """
    print("--- Parsing failure output... ---")
    file_path_matches = re.findall(r'File "(.+?)"', test_output)
    func_name_matches = re.findall(r'File ".+?", line \d+, in (.+)', test_output)
    error_summary_match = re.search(r"\[FAIL\].+?Traceback:.+?ValueError:.+?\n", test_output, re.DOTALL)

    if not file_path_matches or not func_name_matches or not error_summary_match:
        print("Could not parse the failure output.")
        return None, None, None

    failing_file_path = file_path_matches[-1]
    failing_function_name = func_name_matches[-1]
    full_error_text = error_summary_match.group(0)

    print(f"Detected failing file: {failing_file_path}")
    print(f"Detected failing function: {failing_function_name}")
    return failing_file_path, failing_function_name, full_error_text

def parse_llm_output(llm_output):
    """Extracts the code block from the LLM's output."""
    match = re.search(r"\[CODE\](.*)\[/CODE\]", llm_output, re.DOTALL)
    if match:
        return match.group(1).strip()
    return None

def main():
    """
    Main entry point for the self-healing pipeline.
    """
    test_result = run_tests()

    if test_result.returncode == 0:
        print("\n✅ All tests passed. No healing required.")
        sys.exit(0)

    print("\n❌ Tests failed. Initiating self-healing protocol...")
    failing_file, failing_func, error_text = parse_failure(test_result.stdout)

    if not failing_file or not failing_func:
        print("Could not identify failing file or function. Aborting.")
        sys.exit(1)

    original_code = extract_function_code(failing_file, failing_func)
    if not original_code:
        print(f"Could not extract source for function '{failing_func}'. Aborting.")
        sys.exit(1)

    # --- AI-Powered Self-Healing (Simulated) ---
    print("--- Simulating LLM call to get a fix ---")
    llm_output = f"""
[CODE]
def softmax(x, axis=-1, temperature=1.0):
    if temperature <= 0:
        raise ValueError("Temperature must be positive")
    x = x / temperature
    e_x = np.exp(x - np.max(x, axis=axis, keepdims=True))
    return e_x / e_x.sum(axis=axis, keepdims=True)
[/CODE]
"""
    print("--- TissLM Simulated Output ---")
    print(llm_output)
    print("-----------------------------")

    fixed_code = parse_llm_output(llm_output)
    if not fixed_code:
        print("Could not parse the fix from the LLM output. Aborting.")
        sys.exit(1)

    print("--- Parsed Fixed Code ---")
    print(fixed_code)
    print("-------------------------")

    # --- Apply and Verify the Fix ---
    print(f"--- Applying patch to {failing_file} ---")
    try:
        with open(failing_file, 'r') as f:
            full_original_code = f.read()

        # Replace the buggy function with the fixed one
        patched_code = full_original_code.replace(original_code, fixed_code)

        # Write the patched code back to the file
        with open(failing_file, 'w') as f:
            f.write(patched_code)

        print("Patch applied. Re-running tests to verify the fix...")

        # Re-run tests
        verification_result = run_tests()

        if verification_result.returncode == 0:
            print("\n✅✅✅ Self-healing successful! All tests passed after the fix.")
            # As this is a demo, we restore the original buggy code
            print(f"--- Restoring original file {failing_file} for demonstration purposes. ---")
            with open(failing_file, 'w') as f:
                f.write(full_original_code)
            sys.exit(0)
        else:
            print("\n❌❌❌ Self-healing failed. Tests still fail after applying the patch.")

    except Exception as e:
        print(f"\nAn error occurred during patching or verification: {e}")

    finally:
        # Restore the original code to ensure the file is not left in a broken state
        print(f"--- Restoring original file {failing_file} ---")
        if 'full_original_code' in locals():
            with open(failing_file, 'w') as f:
                f.write(full_original_code)
        sys.exit(1)

if __name__ == "__main__":
    main()
