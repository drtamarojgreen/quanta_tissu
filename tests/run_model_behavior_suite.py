import unittest
import os
import sys

# Add project root to path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__)))
# sys.path.insert(0, project_root) # This might not be needed if we change CWD

def run_all_model_behavior_tests():
    # Store original CWD
    original_cwd = os.getcwd()
    # Change CWD to project_root
    os.chdir(project_root)

    features_dir = os.path.join('tests', 'model', 'behavior', 'features') # Relative path now
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()

    print(f"Running tests from: {features_dir} (relative to {os.getcwd()})")

    try:
        # Discover tests in the features directory
        suite = loader.discover(start_dir=features_dir, pattern='*.py')

        # Create a test runner
        runner = unittest.TextTestRunner(verbosity=2)

        # Run the tests
        result = runner.run(suite)
        return result
    finally:
        # Restore original CWD
        os.chdir(original_cwd)

if __name__ == '__main__':
    run_all_model_behavior_tests()
