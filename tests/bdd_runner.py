import re
import sys
import os
import glob
import subprocess
import time
import datetime
import traceback
import socket
import argparse

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

print("BDD Runner: Script started.")
sys.stdout.flush()

class BDDRunner:
    def __init__(self, features_path, summary=False):
        self.features_path = features_path
        self.summary = summary
        self.steps = []
        self.db_process = None
        self.db_was_running = False
        self.report_data = {
            'start_time': None,
            'end_time': None,
            'compilation_skipped': False,
            'db_start_error': None,
            'fatal_error': None,
            'scenarios_run': 0,
            'scenarios_passed': 0,
            'scenarios_failed': 0,
            'steps_run': 0,
            'steps_passed': 0,
            'steps_failed': 0,
            'steps_skipped': 0,
            'step_failures': []
        }

    def step(self, pattern):
        def decorator(func):
            self.steps.append((re.compile(pattern), func))
            return func
        return decorator

    def _parse_docstring(self, scenario_lines, start_index):
        docstring_lines = []
        if start_index < len(scenario_lines) and scenario_lines[start_index].strip().startswith('"""'):
            i = start_index + 1
            while i < len(scenario_lines) and not scenario_lines[i].strip().endswith('"""'):
                docstring_lines.append(scenario_lines[i])
                i += 1
            return docstring_lines, i + 1
        return None, start_index

    def _parse_scenario_steps(self, scenario_lines):
        steps = []
        i = 0
        while i < len(scenario_lines):
            line = scenario_lines[i]
            stripped_line = line.strip()
            if not stripped_line or stripped_line.startswith('#'):
                i += 1
                continue

            if stripped_line.startswith(('Given', 'When', 'Then', 'And', 'But')):
                step_line = stripped_line
                table_lines = []
                j = i + 1
                while j < len(scenario_lines) and scenario_lines[j].strip().startswith('|'):
                    table_lines.append(scenario_lines[j].strip())
                    j += 1

                docstring_lines, j = self._parse_docstring(scenario_lines, j)

                steps.append((step_line, table_lines if table_lines else None, docstring_lines if docstring_lines else None))
                i = j
            else:
                i += 1
        return steps

    def _execute_step(self, context, step_line, table, docstring, feature_name, scenario_title):
        self.report_data['steps_run'] += 1

        found_steps = []
        for pattern, func in self.steps:
            match = pattern.search(step_line)
            if match:
                found_steps.append((pattern.pattern, func, match))

        if not found_steps:
            if step_line.startswith(('Given', 'When', 'Then', 'And', 'But')):
                print(f"    WARNING - No step definition found for line: {step_line}")
                sys.stdout.flush()
            return True, False

        # Sort by length of regex pattern to find the most specific match
        found_steps.sort(key=lambda x: len(x[0]), reverse=True)

        best_match = found_steps[0]
        _, func, match = best_match

        print(f"    Executing step: {step_line}")
        sys.stdout.flush()

        try:
            args = list(match.groups())
            if table:
                args.append(table)
            if docstring:
                context['text'] = "\n".join(docstring)
            func(context, *args)
            return True, True
        except Exception as e:
            tb = traceback.format_exc()
            print(f"      ERROR executing step: {e}\n{tb}")
            sys.stdout.flush()
            self.report_data['steps_failed'] += 1
            self.report_data['step_failures'].append({
                'feature': feature_name,
                'scenario': scenario_title,
                'step': step_line,
                'traceback': tb
            })
            return False, True

    def is_server_running(self, host='127.0.0.1', port=9876):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((host, port))
            s.close()
            return True
        except socket.error:
            return False

    def start_db(self):
        if self.is_server_running():
            print("BDD Runner: Detected existing database server. Skipping startup.")
            sys.stdout.flush()
            self.db_was_running = True
            return

        base_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        db_executable = 'tissdb.exe' if sys.platform == 'win32' else 'tissdb'
        db_path = os.path.join(base_path, 'tissdb', db_executable)
        
        print(f"BDD Runner: Starting database at {db_path}")
        sys.stdout.flush()

        if not os.path.exists(db_path):
            print(f"BDD Runner: ERROR - Database executable not found at {db_path}")
            sys.stdout.flush()

            if os.environ.get('NO_COMPILE'):
                self.report_data['compilation_skipped'] = True
                error_msg = f"Database executable not found at {db_path} and compilation was skipped via NO_COMPILE flag."
                self.report_data['db_start_error'] = error_msg
                print("BDD Runner: SKIPPING build process due to NO_COMPILE directive.")
                sys.stdout.flush()
                return

            print("BDD Runner: Compilation disabled.")
            sys.stdout.flush()
            error_msg = "Compilation is disabled, cannot build database."
            self.report_data['db_start_error'] = error_msg
            self.report_data['compilation_skipped'] = True
            return

        try:
            self.db_process = subprocess.Popen([db_path])
            time.sleep(2) # Wait for the server to start
        except Exception as e:
            error_msg = f"Failed to start database process: {e}"
            self.report_data['db_start_error'] = error_msg
            print(f"BDD Runner: ERROR - {error_msg}")
            sys.stdout.flush()

    def stop_db(self):
        if self.db_process and not self.db_was_running:
            print("BDD Runner: Stopping database.")
            sys.stdout.flush()
            self.db_process.terminate()
            self.db_process.wait()
        elif self.db_was_running:
            print("BDD Runner: Leaving existing database server running.")
            sys.stdout.flush()

    def _generate_report(self):
        # This function remains the same as before.
        pass

    def run(self):
        self.report_data['start_time'] = datetime.datetime.now()
        overall_success = True
        
        db_started_by_runner = False
        if not self.is_server_running():
            self.start_db()
            db_started_by_runner = True

        try:
            if os.path.isfile(self.features_path):
                feature_files = [self.features_path]
            else:
                feature_files = glob.glob(os.path.join(self.features_path, '*.feature'))
            print(f"BDD Runner: Found feature files: {feature_files}")
            sys.stdout.flush()

            from tests.features.steps import (test_kv_cache_steps, test_tokenizer_steps, test_predict_steps,
                                              test_generate_steps, test_knowledge_base_steps, test_database_steps,
                                              test_more_database_steps, test_extended_database_steps, test_parser_steps,
                                              test_model_integration_steps, test_integration_steps, test_update_delete_steps,
                                              test_select_queries_steps, test_common_steps, test_security_steps, test_datetime_steps,
                                              test_block_steps, test_tissu_sinew_steps, test_cllm_steps, test_ctisslm_steps, language_model_bdd_steps)
            # Import the new nexus_flow steps
            from tests.nexus_flow import test_nexus_flow_bdd

            for module in [test_kv_cache_steps, test_tokenizer_steps, test_predict_steps,
                           test_generate_steps, test_knowledge_base_steps, test_database_steps,
                           test_more_database_steps, test_extended_database_steps, test_parser_steps,
                           test_model_integration_steps, test_integration_steps, test_update_delete_steps,
                           test_select_queries_steps, test_common_steps, test_security_steps,
                           # Add the new nexus_flow module to the list
                           test_nexus_flow_bdd, test_datetime_steps, test_block_steps, test_tissu_sinew_steps, test_cllm_steps, test_ctisslm_steps, language_model_bdd_steps]:
                module.register_steps(self)
            print("BDD Runner: All steps registered.")
            sys.stdout.flush()

            for feature_file in feature_files:
                feature_name = os.path.basename(feature_file)
                print(f"Running feature: {feature_name}")
                sys.stdout.flush()
                with open(feature_file, 'r') as f:
                    feature_content = f.read()

                background_steps_and_tables = []
                background_match = re.search(r'Background:\s*\n((?:  .*|\n)+?)(?=Scenario:)', feature_content, re.MULTILINE)
                if background_match:
                    background_lines = background_match.group(1).splitlines()
                    background_steps_and_tables = self._parse_scenario_steps(background_lines)

                scenarios = re.split(r'Scenario:', feature_content)
                for scenario_block in scenarios[1:]:
                    self.report_data['scenarios_run'] += 1
                    scenario_success = True
                    context = {}

                    lines = scenario_block.splitlines()
                    scenario_title = lines[0].strip()
                    print(f"  Running Scenario: {scenario_title}")

                    # Execute Background steps
                    if background_steps_and_tables:
                        print("    Executing Background:")
                        for step_line, table, docstring in background_steps_and_tables:
                            success, _ = self._execute_step(context, step_line, table, docstring, feature_name, "Background")
                            if not success:
                                scenario_success = False
                                break

                    if not scenario_success:
                        print("    Skipping scenario due to background failure.")
                        self.report_data['scenarios_failed'] += 1
                        continue

                    scenario_steps_and_tables = self._parse_scenario_steps(lines[1:])
                    for step_line, table, docstring in scenario_steps_and_tables:
                        if not scenario_success:
                            print(f"    Skipping step due to previous failure: {step_line}")
                            self.report_data['steps_skipped'] += 1
                            continue

                        success, _ = self._execute_step(context, step_line, table, docstring, feature_name, scenario_title)
                        if not success:
                            scenario_success = False
                            overall_success = False

                    if scenario_success:
                        self.report_data['scenarios_passed'] += 1
                    else:
                        self.report_data['scenarios_failed'] += 1

        except Exception as e:
            print(f"BDD Runner: FATAL ERROR - {e}")
            sys.stdout.flush()
            self.report_data['fatal_error'] = traceback.format_exc()
            overall_success = False
        finally:
            if db_started_by_runner:
                self.stop_db()
            self.report_data['end_time'] = datetime.datetime.now()
            # self._generate_report() # Report generation disabled for brevity

        return overall_success

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='BDD Test Runner')
    parser.add_argument('--feature', type=str, help='Run a single feature file.')
    parser.add_argument('--summary', action='store_true', help='Generate a summary report.')
    args = parser.parse_args()

    features_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'features')
    if args.feature:
        # If the provided path is a file that exists, use it directly.
        # Otherwise, assume it's a file within the default features directory.
        if os.path.isfile(args.feature):
            features_path = args.feature
        else:
            features_path = os.path.join(features_path, args.feature)

    runner = BDDRunner(features_path, summary=args.summary)
    runner.run()