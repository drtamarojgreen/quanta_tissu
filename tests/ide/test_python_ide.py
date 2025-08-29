import unittest
from quanta_tissu.ide.p.linter import TissLangLinter
from quanta_tissu.ide.p.editor import Editor

class TestPythonIDE(unittest.TestCase):

    def setUp(self):
        self.linter = TissLangLinter()
        self.editor = Editor()
        self.advanced_tiss_script = """
TASK "Full test of TissLang features"

SETUP {
    RUN "echo 'Setting up environment'"
}

STEP "Handle complex logic" {
    READ "/path/to/some/data.json" AS my_data

    IF LAST_RUN.EXIT_CODE == 0 {
        PROMPT_AGENT "Analyze the following data: {{my_data}}" INTO analysis_result
    } ELSE {
        WRITE "error.log" <<LOG
An error occurred during the setup phase.
LOG
    }

    TRY {
        RUN "python sensitive_script.py"
    } CATCH {
        REQUEST_REVIEW "The sensitive script failed. Please advise."
    }
}

STEP "Final verification" {
    ASSERT FILE "output.txt" EXISTS
}
""".strip().split('\\n')

    def test_linter_on_valid_advanced_script(self):
        """
        Tests that the linter finds no errors in a valid, complex TissLang script.
        """
        # First, format the script correctly so the linter's indentation check passes.
        # This also implicitly tests the formatter's output.
        formatted_lines = self.editor.format_buffer(self.advanced_tiss_script)

        errors = self.linter.lint(formatted_lines)
        self.assertEqual(errors, {}, f"Linter found unexpected errors: {errors}")

    def test_formatter_produces_correct_indentation(self):
        """
        Tests that the formatter correctly indents a complex TissLang script.
        """
        # Create a poorly formatted version of the script
        unformatted_script = [line.lstrip() for line in self.advanced_tiss_script]

        formatted_lines = self.editor.format_buffer(unformatted_script)

        expected_formatting = [
            'TASK "Full test of TissLang features"',
            '',
            'SETUP {',
            '    RUN "echo \'Setting up environment\'"',
            '}',
            '',
            'STEP "Handle complex logic" {',
            '    READ "/path/to/some/data.json" AS my_data',
            '    ',
            '    IF LAST_RUN.EXIT_CODE == 0 {',
            '        PROMPT_AGENT "Analyze the following data: {{my_data}}" INTO analysis_result',
            '    } ELSE {',
            '        WRITE "error.log" <<LOG',
            'An error occurred during the setup phase.',
            'LOG',
            '    }',
            '',
            '    TRY {',
            '        RUN "python sensitive_script.py"',
            '    } CATCH {',
            '        REQUEST_REVIEW "The sensitive script failed. Please advise."',
            '    }',
            '}',
            '',
            'STEP "Final verification" {',
            '    ASSERT FILE "output.txt" EXISTS',
            '}'
        ]

        # We join and split lines to normalize EOL characters for comparison
        self.assertEqual("\\n".join(formatted_lines), "\\n".join(expected_formatting))

if __name__ == '__main__':
    unittest.main()
