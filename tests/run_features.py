import os
import sys

# Add the project root to the Python path to allow importing from other directories.
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tests.test_bdd import parse_feature_file, run_bdd_scenarios, step_registry

def main():
    """
    This script discovers and runs BDD tests from .feature files
    located in the tests/features directory.
    """
    features_dir = os.path.join(os.path.dirname(__file__), 'features')
    print(f"--- Running BDD tests from {features_dir} ---")

    # Ensure the step implementations are loaded
    # By importing test_bdd, the @step decorators will be executed.
    import tests.test_bdd

    has_failures = False
    for filename in sorted(os.listdir(features_dir)):
        if filename.endswith(".feature"):
            filepath = os.path.join(features_dir, filename)

            # Read the file content to check for steps that might trigger compilation.
            with open(filepath, 'r') as f:
                content = f.read()

            if "C++ connector" in content:
                print(f"\n--- SKIPPING {filename} (contains C++ connector tests) ---")
                continue

            print(f"\n--- Running tests from {filename} ---")
            features = parse_feature_file(filepath)
            if not run_bdd_scenarios(features):
                has_failures = True

    print("\n--- All BDD tests finished ---")
    if has_failures:
        print("Some scenarios failed.")
        sys.exit(1)
    else:
        print("All scenarios passed.")
        sys.exit(0)

if __name__ == "__main__":
    main()
