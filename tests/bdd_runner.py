import re
import sys
import os
import glob
import subprocess
import time

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

print("BDD Runner: Script started.")
sys.stdout.flush()

class BDDRunner:
    def __init__(self, features_path):
        self.features_path = features_path
        self.steps = []
        self.db_process = None

    def step(self, pattern):
        def decorator(func):
            self.steps.append((re.compile(pattern), func))
            return func
        return decorator

    def start_db(self):
        base_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        db_executable = 'tissdb.exe' if sys.platform == 'win32' else 'tissdb'
        db_path = os.path.join(base_path, 'tissdb', db_executable)
        
        print(f"BDD Runner: Starting database at {db_path}")
        sys.stdout.flush()

        if not os.path.exists(db_path):
            print(f"BDD Runner: ERROR - Database executable not found at {db_path}")
            sys.stdout.flush()
            # Attempt to build the database
            print("BDD Runner: Attempting to build the database...")
            sys.stdout.flush()
            try:
                subprocess.run(['make', '-C', os.path.join(base_path, 'tissdb')], check=True)
                if not os.path.exists(db_path):
                    print("BDD Runner: ERROR - Build completed but executable still not found.")
                    sys.stdout.flush()
                    return
                print("BDD Runner: Build successful, continuing...")
                sys.stdout.flush()
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                print(f"BDD Runner: ERROR - Failed to build database: {e}")
                sys.stdout.flush()
                return

        self.db_process = subprocess.Popen([db_path])
        time.sleep(2) # Wait for the server to start

    def stop_db(self):
        if self.db_process:
            print("BDD Runner: Stopping database.")
            sys.stdout.flush()
            self.db_process.terminate()
            self.db_process.wait() # Ensure process is terminated

    def run(self):
        self.start_db()
        overall_success = True

        feature_files = glob.glob(os.path.join(self.features_path, '*.feature'))
        print(f"BDD Runner: Found feature files: {feature_files}")
        sys.stdout.flush()
        
        # Import and register steps once
        from tests.features.steps import test_kv_cache_steps
        from tests.features.steps import test_tokenizer_steps
        from tests.features.steps import test_predict_steps
        from tests.features.steps import test_generate_steps
        from tests.features.steps import test_knowledge_base_steps
        from tests.features.steps import test_database_steps
        from tests.features.steps import test_more_database_steps
        from tests.features.steps import test_extended_database_steps
        from tests.features.steps import test_parser_steps
        from tests.features.steps import test_model_integration_steps
        test_kv_cache_steps.register_steps(self)
        test_tokenizer_steps.register_steps(self)
        test_predict_steps.register_steps(self)
        test_generate_steps.register_steps(self)
        test_knowledge_base_steps.register_steps(self)
        test_database_steps.register_steps(self)
        test_more_database_steps.register_steps(self)
        test_extended_database_steps.register_steps(self)
        test_parser_steps.register_steps(self)
        test_model_integration_steps.register_steps(self)
        print("BDD Runner: All steps registered.")
        sys.stdout.flush()

        if not feature_files:
            print("BDD Runner: No feature files found to run.")
            sys.stdout.flush()
            return True # No features is not a failure

        for feature_file in feature_files:
            print(f"Running feature: {os.path.basename(feature_file)}")
            sys.stdout.flush()
            with open(feature_file, 'r') as f:
                feature_content = f.read()
            print(f"BDD Runner: Read content from {os.path.basename(feature_file)}")
            sys.stdout.flush()

            context = {}
            scenario_success = True
            for line_num, line in enumerate(feature_content.splitlines()):
                original_line = line
                line = line.strip()
                
                if not line or line.startswith('#') or line.startswith('Feature:'):
                    continue

                if line.startswith('Scenario:'):
                    context = {} # Reset context for new scenario
                    scenario_success = True

                step_found = False
                for pattern, func in self.steps:
                    match = pattern.match(line)
                    if match:
                        print(f"  Executing step (line {line_num + 1}): {original_line.strip()}")
                        sys.stdout.flush()
                        try:
                            func(context, *match.groups())
                        except Exception as e:
                            print(f"    ERROR executing step: {e}")
                            sys.stdout.flush()
                            scenario_success = False
                            overall_success = False
                        step_found = True
                        break
                if not step_found and line.startswith(('Given', 'When', 'Then', 'And')):
                    print(f"BDD Runner: WARNING - No step definition found for line: {original_line.strip()}")
                    sys.stdout.flush()
        
        self.stop_db()
        return overall_success

if __name__ == '__main__':
    features_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'features')
    runner = BDDRunner(features_path)
    runner.run()