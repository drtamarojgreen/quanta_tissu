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
    print("Discovering and Running Unit Tests...\n") # Added newline for better formatting
    print("=" * 70)

    loader = unittest.TestLoader()
    all_tests_suite = unittest.TestSuite()

    # List of all unit test files in tests/unit, as clarified by the user
    model_test_files = [
        'test_bpe_trainer.py',
        'test_execution_engine.py',
        'test_layers.py',
        'test_model_error_handler.py',
        'test_model_extended.py',
        'test_retrieval_strategies.py',
        'test_tisslang_parser.py',
        'test_tisslm_generation.py',
        'test_tisslm_model.py',
        'test_tokenizer.py',
        'test_train_bpe.py',
        'test_validators.py',
    ]

    for test_file in model_test_files:
        file_path = os.path.join(unit_tests_dir, test_file)
        if os.path.exists(file_path):
            # Load module from file path
            spec = importlib.util.spec_from_file_location(test_file[:-3], file_path)
            module = importlib.util.module_from_spec(spec)
            sys.modules[test_file[:-3]] = module
            spec.loader.exec_module(module)
            
            # Add tests from the loaded module to the suite
            all_tests_suite.addTests(loader.loadTestsFromModule(module))
        else:
            print(f"Warning: Test file not found: {file_path}") # Changed from "Model test file"

    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    results = runner.run(all_tests_suite)

    print("\n" + "=" * 70) # Added newline for better formatting
    if results.wasSuccessful():
        print("All tests passed successfully!")
    else:
        print("Some tests failed or encountered errors.")
    print("=" * 70)

    return results.wasSuccessful()

if __name__ == "__main__":
    discover_and_run_tests()
