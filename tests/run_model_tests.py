import os
import sys
import unittest

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

    if not os.path.isdir(unit_tests_dir):
        print(f"Error: Test directory '{unit_tests_dir}' not found.")
        return False

    print("=" * 70)
    print("Discovering and Running Unit Tests...")
    print("=" * 70)

    # Discover tests
    # start_dir is the directory to start searching for tests
    # pattern is the filename pattern for test files
    loader = unittest.TestLoader()
    suite = loader.discover(start_dir=unit_tests_dir, pattern='test_*.py')

    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    results = runner.run(suite)

    print("=" * 70)
    if results.wasSuccessful():
        print("All tests passed successfully!")
    else:
        print("Some tests failed or encountered errors.")
    print("=" * 70)

    return results.wasSuccessful()

if __name__ == "__main__":
    discover_and_run_tests()