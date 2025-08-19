import numpy as np
import sys
import os

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

if __name__ == '__main__':
    # Example usage of the NLQTranslator
    translator = NLQTranslator()

    # Example 1: Simple query without schema
    print("\n--- Example 1: Simple Query ---")
    question1 = "how many records are in the telemetry collection"
    tissql1 = translator.translate(question1)
    print(f"Question: '{question1}'")
    print(f"TissQL: {tissql1}")

    # Example 2: Query with schema context
    print("\n--- Example 2: Query with Schema ---")
    db_schema = {
        "employees": ["id", "first_name", "last_name", "salary", "department_id"],
        "departments": ["id", "dept_name"]
    }
    question2 = "What is the average salary of employees in department 5?"
    tissql2 = translator.translate(question2, schema=db_schema)
    print(f"Question: '{question2}'")
    print(f"TissQL: {tissql2}")

    # Example 3: More complex query
    print("\n--- Example 3: Complex Query ---")
    question3 = "show the last name of the top 3 highest paid employees"
    tissql3 = translator.translate(question3, schema=db_schema)
    print(f"Question: '{question3}'")
    print(f"TissQL: {tissql3}")
