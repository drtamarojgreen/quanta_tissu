import os
from .test_bdd import parse_feature_file, run_bdd_scenarios

def main():
    # Construct the path to the feature file relative to this script
    features_file = os.path.join(os.path.dirname(__file__), '..', 'docs', 'test.md')

    print(f"--- Running BDD tests from {features_file} ---")
    features = parse_feature_file(features_file)
    run_bdd_scenarios(features)
    print("--- BDD tests finished ---")

if __name__ == "__main__":
    main()
