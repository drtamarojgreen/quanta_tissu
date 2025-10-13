import numpy as np
import sys
import os
import subprocess
import json

# Adjust the path to import from the parent directory's 'tisslm' module
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from tisslm.model import QuantaTissu
from tisslm.tokenizer import tokenize, detokenize
from tisslm.config import model_config, system_config

class NLQTranslator:
    """
    A Natural Language to TissQL translator using the QuantaTissu LLM.
    """
    def __init__(self):
        """
        Initializes the translator, loading the QuantaTissu model and tokenizer.
        """
        print("Initializing NLQTranslator...")
        # For reproducibility, let's seed numpy's random generator
        np.random.seed(42)
        # We can load the model without the DB knowledge base for this task
        self.model = QuantaTissu(model_config, use_db=False)
        # Load the trained weights
        model_path = system_config["model_save_path"]
        self.model.load_weights(model_path)
        print("NLQTranslator initialized successfully.")

    def _create_prompt(self, question: str, schema: dict = None) -> str:
        """
        Creates a detailed few-shot prompt for the LLM.
        """
        prompt = f"""You are an expert TissQL translator. Your task is to convert the user's question into a valid TissQL query.

### TissQL Syntax Reference
- SELECT: `SELECT [* | field1, AGG(field2)] FROM collection [WHERE ...] [GROUP BY ...] [ORDER BY ...] [LIMIT ...]`
- INSERT: `INSERT INTO collection (col1, col2) VALUES (val1, val2)`
- UPDATE: `UPDATE collection SET col1 = val1 [WHERE ...]`
- DELETE: `DELETE FROM collection [WHERE ...]`
- JOIN: `... [INNER|LEFT|RIGHT] JOIN collection ON condition ...`
- Aggregates: `COUNT`, `SUM`, `AVG`, `MIN`, `MAX`, `STDDEV`
- Operators: `=`, `!=`, `<`, `>`, `<=`, `>=`, `LIKE`, `AND`, `OR`

"""

        if schema:
            prompt += "### Database Schema\n"
            for table, columns in schema.items():
                prompt += f"- {table}: {', '.join(columns)}\n"
            prompt += "\n"

        prompt += """### Examples
Question: "How many users are there?"
TissQL: SELECT COUNT(*) FROM users

Question: "Show me the emails of users who signed up after the start of 2023"
TissQL: SELECT email FROM users WHERE signup_date > '2023-01-01'

Question: "List the 10 most recent orders"
TissQL: SELECT * FROM orders ORDER BY order_date DESC LIMIT 10

### Task
Translate the following question into a single TissQL query. Do not add any explanation or conversational text.

Question: "{question}"
TissQL: """
        return prompt

    def translate(self, question: str, schema: dict = None, n_new_tokens=50, method="greedy") -> str:
        """
        Translates a natural language question into a TissQL query.

        Args:
            question: The natural language question to translate.
            schema: A dictionary describing the database schema (e.g., {"table_name": ["col1", "col2"]}).
            n_new_tokens: The maximum number of tokens to generate for the query.
            method: The generation method (e.g., "greedy", "top_k").

        Returns:
            The translated TissQL query string.
        """
        full_prompt = self._create_prompt(question, schema)
        prompt_tokens = tokenize(full_prompt)

        # Check if the token array is empty. The check needs to be compatible with numpy arrays.
        if prompt_tokens is None or len(prompt_tokens) == 0:
            print("Warning: Prompt resulted in empty token sequence.")
            return "ERROR: Could not tokenize the prompt."

        generated_tokens = self.model.generate(
            prompt_tokens,
            n_new_tokens=n_new_tokens,
            method=method
        )

        tissql_query = detokenize(generated_tokens).strip()

        # The model might generate text beyond the query, so we clean it up.
        # A simple way is to stop at the first semicolon or newline if they exist.
        tissql_query = tissql_query.split(';')[0].split('\n')[0]

        return tissql_query

    def execute_on_lite(self, tissql_query: str) -> dict:
        """
        Executes a TissQL query on the tissdb-lite Node.js script.

        Args:
            tissql_query: The TissQL query to execute.

        Returns:
            A dictionary containing the query result or an error message.
        """
        if not tissql_query:
            return {"error": "Cannot execute an empty query."}

        # The node script is in the `lite` directory, relative to the repo root.
        # We need to construct the path to it.
        # __file__ is quanta_tissu/analytics/translator.py
        # We need to go up two levels to the repo root.
        repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
        script_path = os.path.join(repo_root, 'lite', 'query_executor.js')

        if not os.path.exists(script_path):
            return {"error": f"Query executor not found at {script_path}"}

        process = None
        try:
            process = subprocess.Popen(
                ['node', script_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                encoding='utf-8'
            )

            stdout, stderr = process.communicate(input=tissql_query + '\\n', timeout=10)

            if process.returncode != 0:
                return {"error": "Query executor script failed", "details": stderr}

            # The script might return an error object or an array of results.
            try:
                result = json.loads(stdout)
                return result
            except json.JSONDecodeError:
                return {"error": "Failed to decode JSON from query executor", "raw_output": stdout}

        except subprocess.TimeoutExpired:
            if process:
                process.kill()
            return {"error": "Query execution timed out."}
        except Exception as e:
            return {"error": f"An unexpected error occurred: {str(e)}"}


if __name__ == '__main__':
    # Example usage of the NLQTranslator
    # Note: This requires the environment to be fully set up first.
    # See docs/tisslm_pipeline_implementation.md for instructions.

    # 1. Install dependencies
    # pip install -r requirements.txt

    # 2. Create the corpus file (or use an existing one)
    # See documentation for details.

    # 3. Train the tokenizer
    # python3 -m quanta_tissu.tisslm.train_bpe

    # 4. Train the model
    # python3 -m quanta_tissu.tisslm.train

    print("Running NLQTranslator example...")
    translator = NLQTranslator()

    print("\n--- Example 1: Translate and Execute ---")
    question1 = "show me all developers"
    print(f"Question: '{question1}'")

    # 1. Translate NLQ to TissQL
    tissql1 = translator.translate(question1, schema={"users": ["name", "age", "role", "status"]})
    print(f"Translated TissQL: {tissql1}")

    # 2. Execute the TissQL on tissdb-lite
    print("\nExecuting query on tissdb-lite...")
    results = translator.execute_on_lite(tissql1)

    # 3. Print the results
    print("Execution Results:")
    if "error" in results:
        print(f"  Error: {results['error']}")
        if "details" in results:
            print(f"  Details: {results['details']}")
    else:
        # Using json.dumps for pretty printing the list of dicts
        print(json.dumps(results, indent=2))
