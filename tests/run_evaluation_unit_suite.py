import unittest
import os
import sys

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..'))
sys.path.insert(0, project_root)

def run_tests():
    """
    Discovers and runs all unit tests in the tests/evaluation directory.
    """
    print("\n" + "="*40)
    print("Running Evaluation Unit Tests")
    print("="*40)

    # Discover tests in the tests/evaluation directory
    test_dir = os.path.join(os.path.dirname(__file__), 'evaluation')
    if not os.path.exists(test_dir):
        print(f"Error: Test directory not found: {test_dir}")
        sys.exit(1)

    loader = unittest.TestLoader()
    suite = loader.discover(start_dir=test_dir, pattern='test_*.py')

    # Run the tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    print("\n" + "="*40)
    print("Evaluation Unit Tests Complete")
    print("="*40)

    if result.wasSuccessful():
        print("All tests passed!")
    else:
        print("Some tests failed.")
        sys.exit(1)

if __name__ == '__main__':
    run_tests()
