import json
from tisslang_parser import TissLangParser, TissLangParserError

if __name__ == '__main__':
    # Example usage: Parse the script from the TissLang plan.
    example_script = """
#TISS! Language=Python

TASK "Create and test a simple Python hello world script"

STEP "Create the main application file" {

    WRITE "main.py" <<PYTHON
import sys

def main():
    print(f"Hello, {sys.argv[1]}!")


if __name__ == "__main__":
    main()
PYTHON
}


STEP "Run the script and verify its output" {
    RUN "python main.py TissLang"
    ASSERT LAST_RUN.EXIT_CODE == 0
    ASSERT LAST_RUN.STDOUT CONTAINS "Hello, TissLang!"
    READ "main.py" AS main_code
}
"""

    print("--- Parsing TissLang Script ---")
    parser = TissLangParser()
    try:
        parsed_ast = parser.parse(example_script)
        print("--- Parse Successful. AST: ---")
        print(json.dumps(parsed_ast, indent=2))
    except TissLangParserError as e:
        print(f"Parsing failed: {e}")
