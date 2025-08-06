import os
import importlib.util
import inspect
import traceback
import time
import sys

def discover_and_run_tests():
    """
    Discovers and runs all tests in the 'tests/' directory.
    """
    tests_dir = 'tests'
    if not os.path.isdir(tests_dir):
        print(f"Error: Test directory '{tests_dir}' not found.")
        return False

    test_files = [f for f in os.listdir(tests_dir) if f.startswith('test_') and f.endswith('.py')]

    passed_count = 0
    failed_count = 0
    failed_tests = []

    print("=" * 70)
    print("Running tests...")
    print("=" * 70)
    start_time = time.time()

    for file_name in test_files:
        module_name = file_name[:-3]
        file_path = os.path.join(tests_dir, file_name)

        try:
            # Dynamically import the module
            spec = importlib.util.spec_from_file_location(module_name, file_path)
            module = importlib.util.module_from_spec(spec)
            # Add the module to sys.modules to handle imports within tests
            sys.modules[module_name] = module
            spec.loader.exec_module(module)

            # Find all test functions in the module
            for name, func in inspect.getmembers(module, inspect.isfunction):
                if name.startswith('test_'):
                    try:
                        func()
                        print(f"PASS: {module_name}.{name}")
                        passed_count += 1
                    except AssertionError as e:
                        print(f"FAIL: {module_name}.{name}")
                        # Get a cleaner traceback
                        tb = traceback.format_exc()
                        clean_tb = "\n".join(tb.splitlines()[3:]) # Hide the runner's frames
                        failed_tests.append((f"{module_name}.{name}", str(e), clean_tb))
                        failed_count += 1
                    except Exception as e:
                        print(f"ERROR: {module_name}.{name}")
                        tb = traceback.format_exc()
                        failed_tests.append((f"{module_name}.{name}", str(e), tb))
                        failed_count += 1
        except Exception as e:
            print(f"CRITICAL ERROR: Could not import or run tests from {file_name}")
            tb = traceback.format_exc()
            failed_tests.append((f"{file_name}", str(e), tb))
            failed_count += 1

    end_time = time.time()
    duration = end_time - start_time

    print("-" * 70)

    if failed_count > 0:
        print("\n--- FAILED TESTS SUMMARY ---")
        for name, error_msg, tb in failed_tests:
            print(f"\n[FAIL] {name}")
            print(f"  Error: {error_msg}")
            print(f"  Traceback:\n{tb}")
        print("-" * 70)

    print(f"Ran {passed_count + failed_count} tests in {duration:.4f}s")
    print(f"\nResult: {passed_count} passed, {failed_count} failed")
    print("=" * 70)

    return failed_count == 0

if __name__ == "__main__":
    # Ensure the script can find the 'quanta_tissu' package
    sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

    success = discover_and_run_tests()
    if not success:
        sys.exit(1)
