import os
import sys
import unittest
import importlib.util # Needed for loading modules by path

def discover_and_run_tests():
    """
    Discovers and runs all unit tests in the 'tests/unit' directory
    using Python's built-in unittest framework.
    """
    base_dir = os.path.dirname(os.path.abspath(__file__))
    unit_tests_dir = os.path.join(base_dir, 'unit')

    # Ensure the project root is in sys.path for package discovery
    # Assuming project root is one level up from 'tests' directory
    project_root = os.path.abspath(os.path.join(base_dir, '..'))
    if project_root not in sys.path:
        sys.path.insert(0, project_root)

    # Add the quanta_tissu package root to sys.path for internal imports
    quanta_tissu_package_root = os.path.abspath(os.path.join(project_root, 'quanta_tissu'))
    if quanta_tissu_package_root not in sys.path:
        sys.path.insert(0, quanta_tissu_package_root)

    # Add the unit tests directory to sys.path for internal imports within tests/unit
    if unit_tests_dir not in sys.path:
        sys.path.insert(0, unit_tests_dir)

    if not os.path.isdir(unit_tests_dir):
        print(f"Error: Test directory '{unit_tests_dir}' not found.")
        return False

    print("=" * 70)
    print("Discovering and Running All Tests (Unit & Behavior)...
")
    print("=" * 70)

    loader = unittest.TestLoader()
    # Discover all tests in the 'tests' directory and its subdirectories.
    # This is more robust than maintaining a hardcoded list of files and will
    # include both unit and behavior tests.
    all_tests_suite = loader.discover(base_dir, pattern='test_*.py')

    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    results = runner.run(all_tests_suite)

    print("\n" + "=" * 70)
    if results.wasSuccessful():
        print("All tests passed successfully!")
    else:
        print("Some tests failed or encountered errors.")
    print("=" * 70)

    return results.wasSuccessful()

if __name__ == "__main__":
    discover_and_run_tests()
