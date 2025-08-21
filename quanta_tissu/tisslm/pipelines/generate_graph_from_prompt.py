import os
import sys
import json
import argparse
import numpy as np

from ..core.model import QuantaTissu
from ..core.tokenizer import Tokenizer
from ..config import model_config

# --- Constants ---
MODELS_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'models')
TOKENIZER_PATH = os.path.join(MODELS_DIR, 'tokenizer.json')
MODEL_PATH = os.path.join(MODELS_DIR, 'quanta_tissu.npz')

SYSTEM_PROMPT = """You are an expert in creating graph diagrams. Your task is to generate a graph based on the user's prompt. You must respond ONLY with a valid JSON object that represents the graph. Do not include any other text, explanations, or markdown. The JSON object must have two keys: "nodes" and "edges".

The "nodes" key must be an array of objects, where each object has an "id" (integer) and a "label" (string).

The "edges" key must be an array of objects, where each object has a "from" (integer) and a "to" (integer), representing the node IDs of the connection.

Example response for the prompt "a simple triangle":
{
  "nodes": [
    {"id": 1, "label": "Point A"},
    {"id": 2, "label": "Point B"},
    {"id": 3, "label": "Point C"}
  ],
  "edges": [
    {"from": 1, "to": 2},
    {"from": 2, "to": 3},
    {"from": 3, "to": 1}
  ]
}

Now, generate a graph for the following prompt.
"""

def generate_graph_json(prompt: str, length: int = 512) -> str:
    """
    Generates a graph in JSON format from a text prompt using the trained model.
    """
    # 1. Check for model files
    if not os.path.exists(TOKENIZER_PATH) or not os.path.exists(MODEL_PATH):
        error_msg = {
            "error": "Model files not found.",
            "message": f"Ensure tokenizer.json and quanta_tissu.npz exist in the '{MODELS_DIR}' directory."
        }
        return json.dumps(error_msg, indent=2)

    # 2. Load tokenizer and model
    try:
        tokenizer = Tokenizer.from_file(TOKENIZER_PATH)
        model = QuantaTissu(model_config)
        model.load_weights(MODEL_PATH)
    except Exception as e:
        error_msg = {
            "error": "Failed to load model or tokenizer.",
            "message": str(e)
        }
        return json.dumps(error_msg, indent=2)

    # 3. Construct the full prompt
    full_prompt = f"{SYSTEM_PROMPT}\nPrompt: \"{prompt}\"\n\nJSON Output:"

    # 4. Generate text
    prompt_token_ids = tokenizer.tokenize(full_prompt)
    generated_ids = model.generate(prompt_token_ids, n_new_tokens=length, method="greedy")
    full_text = tokenizer.detokenize(prompt_token_ids + generated_ids)

    # 5. Extract and validate the JSON from the model's output
    try:
        # Find the start of the JSON object
        json_start = full_text.find('{')
        if json_start == -1:
            raise ValueError("No JSON object found in model output.")

        # Find the end of the JSON object
        json_end = full_text.rfind('}')
        if json_end == -1:
            raise ValueError("Incomplete JSON object in model output.")

        json_str = full_text[json_start : json_end + 1]

        # Validate that it's valid JSON
        parsed_json = json.loads(json_str)

        # Basic validation for graph structure
        if "nodes" not in parsed_json or "edges" not in parsed_json:
            raise ValueError("Generated JSON is missing 'nodes' or 'edges' key.")

        return json.dumps(parsed_json, indent=2)

    except (ValueError, json.JSONDecodeError) as e:
        error_msg = {
            "error": "Failed to parse valid graph JSON from model output.",
            "message": str(e),
            "raw_output": full_text
        }
        return json.dumps(error_msg, indent=2)


def main():
    """
    Main function to parse arguments and run the graph generation.
    """
    parser = argparse.ArgumentParser(description="Generate a graph in JSON format from a prompt.")
    parser.add_argument(
        "prompt",
        type=str,
        help="The text prompt to generate the graph from."
    )
    parser.add_argument(
        "--length",
        type=int,
        default=512,
        help="The maximum number of tokens to generate for the JSON output."
    )
    args = parser.parse_args()

    generated_json = generate_graph_json(args.prompt, args.length)
    print(generated_json)

if __name__ == "__main__":
    main()
