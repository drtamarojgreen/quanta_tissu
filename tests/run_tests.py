import os
import importlib.util
import inspect
import traceback
import time
import sys

from .bdd_runner import BDDRunner

def discover_and_run_tests():
    """
    Discovers and runs all tests in the 'tests/' directory.
    """
    tests_dir = os.path.dirname(os.path.abspath(__file__))
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

    # Run standard tests
    for file_name in test_files:
        module_name = file_name[:-3]
        file_path = os.path.join(tests_dir, file_name)

        try:
            spec = importlib.util.spec_from_file_location(module_name, file_path)
            module = importlib.util.module_from_spec(spec)
            sys.modules[module_name] = module
            spec.loader.exec_module(module)

            for name, func in inspect.getmembers(module, inspect.isfunction):
                if name.startswith('test_'):
                    try:
                        func()
                        print(f"PASS: {module_name}.{name}")
                        passed_count += 1
                    except AssertionError as e:
                        print(f"FAIL: {module_name}.{name}")
                        tb = traceback.format_exc()
                        clean_tb = "\n".join(tb.splitlines()[3:])
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

    # Run BDD tests
    # NOTE: BDD tests are temporarily disabled as they require a compiled
    # database executable, which cannot be built in the current environment
    # due to restrictions on compilation.
    print("-" * 70)
    print("Running BDD tests...")
    features_path = os.path.join(tests_dir, 'features')
    bdd_runner = BDDRunner(features_path)
    if bdd_runner.run():
        passed_count += 1
        print("BDD tests: PASSED")
    else:
        failed_count += 1
        failed_tests.append(("BDD tests", "One or more BDD scenarios failed.", ""))
        print("BDD tests: FAILED")


    end_time = time.time()
    duration = end_time - start_time

    print("-" * 70)

    if failed_count > 0:
        print("\n--- FAILED TESTS SUMMARY ---")
        for name, error_msg, tb in failed_tests:
            print(f"\n[FAIL] {name}")
            print(f"  Error: {error_msg}")
            if tb:
                print(f"  Traceback:\n{tb}")
        print("-" * 70)

    print(f"Ran {passed_count + failed_count} tests in {duration:.4f}s")
    print(f"\nResult: {passed_count} passed, {failed_count} failed")
    print("=" * 70)

    return failed_count == 0

if __name__ == "__main__":
    # Ensure the script can find the 'quanta_tissu' package
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

    discover_and_run_tests()
