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

    def is_server_running(self, host='127.0.0.1', port=8080):
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

            print("BDD Runner: Attempting to build the database...")
            sys.stdout.flush()
            try:
                subprocess.run(['make', '-C', os.path.join(base_path, 'tissdb')], check=True)
                if not os.path.exists(db_path):
                    error_msg = "Build completed but executable still not found."
                    self.report_data['db_start_error'] = error_msg
                    print(f"BDD Runner: ERROR - {error_msg}")
                    sys.stdout.flush()
                    return
                print("BDD Runner: Build successful, continuing...")
                sys.stdout.flush()
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                error_msg = f"Failed to build database: {e}"
                self.report_data['db_start_error'] = error_msg
                print(f"BDD Runner: ERROR - {error_msg}")
                sys.stdout.flush()
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
            self.db_process.wait() # Ensure process is terminated
        elif self.db_was_running:
            print("BDD Runner: Leaving existing database server running.")
            sys.stdout.flush()

    def _generate_summary_report(self):
        report_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'docs', 'tissdb_bdd_summary_report.md')

        duration = (self.report_data['end_time'] - self.report_data['start_time']).total_seconds()

        if self.report_data['fatal_error']:
            overall_result = "FAIL (FATAL)"
        elif self.report_data['scenarios_failed'] > 0 or self.report_data['db_start_error']:
            overall_result = "FAIL"
        else:
            overall_result = "PASS"

        content = f"""# TissDB BDD Test Execution Summary

- **Date:** {self.report_data['start_time'].strftime('%Y-%m-%d %H:%M:%S')}
- **Duration:** {duration:.2f} seconds
- **Overall Result:** {overall_result}

## Summary

| Metric             | Count |
| ------------------ | ----- |
| Scenarios Run      | {self.report_data['scenarios_run']}      |
| Scenarios Passed   | {self.report_data['scenarios_passed']}    |
| Scenarios Failed   | {self.report_data['scenarios_failed']}    |
| Steps Run          | {self.report_data['steps_run']}          |
| Steps Passed       | {self.report_data['steps_passed']}        |
| Steps Failed       | {self.report_data['steps_failed']}        |
| Steps Skipped      | {self.report_data['steps_skipped']}        |

"""
        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, 'w') as f:
            f.write(content)
        print(f"BDD Runner: Summary report generated at {report_path}")

    def _generate_report(self):
        if self.summary:
            self._generate_summary_report()
        else:
            self._generate_detailed_report()

    def _generate_detailed_report(self):
        report_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'docs', 'tissdb_bdd_implementation_plan.md')

        duration = (self.report_data['end_time'] - self.report_data['start_time']).total_seconds()

        if self.report_data['fatal_error']:
            overall_result = "FAIL (FATAL)"
        elif self.report_data['scenarios_failed'] > 0 or self.report_data['db_start_error']:
            overall_result = "FAIL"
        else:
            overall_result = "PASS"

        content = f"""# TissDB BDD Test Execution Report

- **Date:** {self.report_data['start_time'].strftime('%Y-%m-%d %H:%M:%S')}
- **Duration:** {duration:.2f} seconds
- **Overall Result:** {overall_result}

## Summary

| Metric             | Count |
| ------------------ | ----- |
| Scenarios Run      | {self.report_data['scenarios_run']}      |
| Scenarios Passed   | {self.report_data['scenarios_passed']}    |
| Scenarios Failed   | {self.report_data['scenarios_failed']}    |
| Steps Run          | {self.report_data['steps_run']}          |
| Steps Passed       | {self.report_data['steps_passed']}        |
| Steps Failed       | {self.report_data['steps_failed']}        |
| Steps Skipped      | {self.report_data['steps_skipped']}        |

## Details

### Environment
- **Compilation Skipped:** {'Yes' if self.report_data['compilation_skipped'] else 'No'}

### Errors

"""
        if self.report_data['db_start_error']:
            content += f"**Database Start Error:**\n\n```\n{self.report_data['db_start_error']}\n```\n\n"

        if self.report_data['fatal_error']:
            content += f"**Fatal Script Error:**\n\n```\n{self.report_data['fatal_error']}\n```\n\n"

        if self.report_data['step_failures']:
            content += "**Failed Steps:**\n"
            for failure in self.report_data['step_failures']:
                content += f"\n- **Step:** `{failure['step']}`\n"
                content += f"  - **Feature:** `{failure['feature']}`\n"
                content += f"  - **Scenario:** `{failure['scenario']}`\n"
                content += f"  - **Error:**\n    ```\n{failure['traceback']}\n    ```\n"

        content += "\\n## Conclusion\\n\\n"
        if overall_result.startswith("FAIL"):
            content += "The test run failed. See error details above."
        else:
            content += "All scenarios passed successfully."

        os.makedirs(os.path.dirname(report_path), exist_ok=True)
        with open(report_path, 'w') as f:
            f.write(content)
        print(f"BDD Runner: Report generated at {report_path}")

    def run(self):
        self.report_data['start_time'] = datetime.datetime.now()
        overall_success = True
        
        try:
            self.start_db()

            feature_files = glob.glob(os.path.join(self.features_path, '*.feature'))
            print(f"BDD Runner: Found feature files: {feature_files}")
            sys.stdout.flush()

            from tests.features.steps import (test_kv_cache_steps, test_tokenizer_steps, test_predict_steps,
                                              test_generate_steps, test_knowledge_base_steps, test_database_steps,
                                              test_more_database_steps, test_extended_database_steps, test_parser_steps,
                                              test_model_integration_steps, test_integration_steps, test_update_delete_steps,
                                              test_select_queries_steps)
            for module in [test_kv_cache_steps, test_tokenizer_steps, test_predict_steps,
                           test_generate_steps, test_knowledge_base_steps, test_database_steps,
                           test_more_database_steps, test_extended_database_steps, test_parser_steps,
                           test_model_integration_steps, test_integration_steps, test_update_delete_steps,
                           test_select_queries_steps]:
                module.register_steps(self.step)
            print("BDD Runner: All steps registered.")
            sys.stdout.flush()

            if not feature_files:
                print("BDD Runner: No feature files found to run.")
                sys.stdout.flush()
                # No features is a success, but we still want to generate a report.
                return True

            for feature_file in feature_files:
                feature_name = os.path.basename(feature_file)
                print(f"Running feature: {feature_name}")
                sys.stdout.flush()
                with open(feature_file, 'r') as f:
                    feature_content = f.read()

                # This logic is simplified to avoid state-related bugs across scenarios.
                # A more robust parser would be better, but this is less invasive.
                scenarios = feature_content.split('Scenario:')
                for i, scenario_block in enumerate(scenarios[1:]): # Skip feature description
                    self.report_data['scenarios_run'] += 1
                    scenario_success = True
                    context = {}

                    lines = scenario_block.splitlines()
                    scenario_title = lines[0].strip()
                    print(f"  Running Scenario: {scenario_title}")

                    for line_num, step_line_full in enumerate(lines[1:]):
                        step_line = step_line_full.strip()

                        if not step_line or step_line.startswith('#'):
                            continue

                        if not scenario_success:
                            print(f"    Skipping step due to previous failure: {step_line}")
                            self.report_data['steps_skipped'] += 1
                            continue

                        self.report_data['steps_run'] += 1
                        step_found = False
                        for pattern, func in self.steps:
                            match = pattern.match(step_line)
                            if match:
                                print(f"    Executing step: {step_line}")
                                sys.stdout.flush()
                                try:
                                    func(context, *match.groups())
                                    self.report_data['steps_passed'] += 1
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
                                    scenario_success = False
                                    overall_success = False
                                step_found = True
                                break
                        if not step_found and step_line.startswith(('Given', 'When', 'Then', 'And', 'But')):
                            print(f"    WARNING - No step definition found for line: {step_line}")
                            sys.stdout.flush()

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
            self.stop_db()
            self.report_data['end_time'] = datetime.datetime.now()
            self._generate_report()

        return overall_success

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='BDD Test Runner')
    parser.add_argument('--summary', action='store_true', help='Generate a summary report.')
    args = parser.parse_args()

    features_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'features')
    runner = BDDRunner(features_path, summary=args.summary)
    runner.run()