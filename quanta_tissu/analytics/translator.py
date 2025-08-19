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

    print("\n--- Example 1: Simple Query ---")
    question1 = "how many records are in the telemetry collection"
    tissql1 = translator.translate(question1)
    print(f"Question: '{question1}'")
    print(f"TissQL: {tissql1}")
