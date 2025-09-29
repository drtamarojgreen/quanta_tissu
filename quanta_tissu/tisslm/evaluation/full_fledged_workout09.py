import os
import sys
import json

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser, TissLangParserError

def run_tisslang_parser_tests():
    report = ["\n--- Test 9: TissLang Parser ---"]
    parser = TissLangParser()

    test_scripts = [
        {
            "name": "Simple STEP command",
            "script": "STEP \"Perform a task\"\n  RUN echo \"Hello\"\n}",
            "expected_type": "STEP",
            "expected_description": "Perform a task",
            "expected_commands_len": 1
        },
        {
            "name": "SETUP command with VAR and LOG",
            "script": "SETUP \"Initialize environment\"\n  VAR my_var = \"test_value\"\n  LOG \"Variable set to: {my_var}\"\n}",
            "expected_type": "SETUP",
            "expected_description": "Initialize environment",
            "expected_commands_len": 2
        },
        {
            "name": "IF-ELSE block",
            "script": "STEP \"Test\" {\nIF condition == true\n  LOG \"Condition is true\"\nELSE\n  LOG \"Condition is false\"\n}",
            "expected_type": "IF_ELSE",
            "expected_condition": "condition == true",
            "expected_then_len": 1,
            "expected_else_len": 1
        },
        {
            "name": "WRITE command (heredoc)",
            "script": "STEP \"Test\" {\nWRITE file.txt <<EOF\nHello World\nThis is a test.\nEOF\n}",
            "expected_type": "WRITE",
            "expected_path": "file.txt",
            "expected_content_start": "Hello World"
        },
        {
            "name": "PROMPT_AGENT command",
            "script": "STEP \"Test\" {\nPROMPT_AGENT \"Summarize this text\" -> summary_var\n}",
            "expected_type": "PROMPT_AGENT",
            "expected_prompt": "Summarize this text",
            "expected_variable": "summary_var"
        }
    ]

    for i, test_case in enumerate(test_scripts):
        report.append(f"\n  --- Running test case {i+1}: {test_case['name']} ---")
        try:
            ast = parser.parse(test_case['script'])
            report.append(f"  Parsed AST: {json.dumps(ast, indent=2)}")

            # Basic verification (can be expanded)
            if test_case['expected_type'] == "STEP" or test_case['expected_type'] == "SETUP":
                assert ast[0]['type'] == test_case['expected_type']
                assert ast[0]['description'] == test_case['expected_description']
                assert len(ast[0]['commands']) == test_case['expected_commands_len']
                report.append("  Verification: Type, description, and command count match.")
            elif test_case['expected_type'] == "IF_ELSE":
                assert ast[0]['type'] == "IF"
                assert ast[0]['condition'] == test_case['expected_condition']
                assert len(ast[0]['then_block']) == test_case['expected_then_len']
                assert len(ast[0]['else_block']) == test_case['expected_else_len']
                report.append("  Verification: IF-ELSE structure matches.")
            elif test_case['expected_type'] == "WRITE":
                assert ast[0]['type'] == "WRITE"
                assert ast[0]['path'] == test_case['expected_path']
                assert ast[0]['content'].startswith(test_case['expected_content_start'])
                report.append("  Verification: WRITE command structure and content match.")
            elif test_case['expected_type'] == "PROMPT_AGENT":
                assert ast[0]['type'] == "PROMPT_AGENT"
                assert ast[0]['prompt'] == test_case['expected_prompt']
                assert ast[0]['variable'] == test_case['expected_variable']
                report.append("  Verification: PROMPT_AGENT command structure matches.")

            report.append("  Test case PASSED.")

        except TissLangParserError as e:
            report.append(f"  [ERROR] TissLangParserError: {e}")
            report.append(f"  Script:\n{test_case['script']}")
        except Exception as e:
            report.append(f"  [ERROR] Unexpected error during parsing: {e}")
            report.append(f"  Script:\n{test_case['script']}")

    return report

if __name__ == '__main__':
    report = run_tisslang_parser_tests()
    for line in report:
        print(line)
