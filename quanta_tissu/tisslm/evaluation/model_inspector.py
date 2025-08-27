import logging
import numpy as np
from typing import List, Dict, Any, Optional

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.core.model import QuantaTissu
# from quanta_tissu.tisslm.core.tokenizer import Tokenizer
# from quanta_tissu.tisslm.config import model_config


logger = logging.getLogger(__name__)

def inspect_model_forward_pass(model, tokenizer, input_text: str, block_size: int):
    """
    Inspects the forward pass of the model with a given input text.
    Logs intermediate activations if the model supports it.

    Args:
        model: The QuantaTissu model instance.
        tokenizer: The tokenizer instance.
        input_text: The text to pass through the model.
        block_size: The block size (sequence length) the model expects.
    """
    logger.info(f"\n--- Inspecting Model Forward Pass for Input: '{input_text}' ---")

    # 1. Tokenization Step
    logger.info("Step 1: Tokenization")
    try:
        token_ids = tokenizer.tokenize(input_text).tolist()
        # Pad or truncate to block_size
        if len(token_ids) > block_size:
            token_ids = token_ids[:block_size]
            logger.warning(f"Input text truncated to {block_size} tokens.")
        elif len(token_ids) < block_size:
            # Assuming padding with 0s or a specific pad_token_id
            # You might need to adjust this based on your tokenizer's padding
            token_ids = token_ids + [0] * (block_size - len(token_ids))
            logger.info(f"Input text padded to {block_size} tokens.")
        
        input_tensor = np.array([token_ids]) # Batch size of 1
        logger.info(f"Input text encoded to token IDs: {token_ids}")
        logger.info(f"Input tensor shape: {input_tensor.shape}")
    except Exception as e:
        logger.error(f"Error during tokenization: {e}")
        return

    # 2. Embedding Layer Inspection
    logger.info("\nStep 2: Embedding Layer")
    try:
        # Access embeddings directly from the model's Parameter object
        embeddings = model.embeddings.value[input_tensor]
        logger.info(f"Embeddings shape: {embeddings.shape}")
        logger.info(f"Embeddings (first 5 tokens, first 5 dimensions):\n{embeddings[0, :5, :5]}")
    except Exception as e:
        logger.error(f"Error inspecting embedding layer: {e}")
        return # Exit if embeddings fail, as subsequent steps depend on it

    # 2.5 Positional Encoding
    logger.info("\nStep 2.5: Positional Encoding")
    try:
        # Apply positional encoding
        # The diagnostic 'dir(model)' output revealed a nested 'model' attribute.
        # This suggests the main components are inside 'model.model'.
        # We will access the positional encoding and subsequent layers through this nested object.
        x_pos_encoded = model.model.pos_encoding(embeddings, start_pos=0)
        logger.info(f"Positional Encoded shape: {x_pos_encoded.shape}")
        logger.info(f"Positional Encoded (first 5 tokens, first 5 dimensions):\n{x_pos_encoded[0, :5, :5]}")
        current_output = x_pos_encoded
    except Exception as e:
        logger.error(f"Error during positional encoding: {e}")
        return # Exit if positional encoding fails

    # 3. Transformer Block Inspection
    logger.info("\nStep 3: Transformer Blocks")
    # Iterate through each transformer block and log its output
    # Accessing transformer_blocks through the nested model object
    for i, block in enumerate(model.model.transformer_blocks):
        try:
            # Pass the current_output through the block
            # The block returns a tuple (output_tensor, kv_cache). For this inspection,
            # we only need the output tensor, which is the first element of the tuple.
            result_tuple = block(current_output, mask=None, kv_cache=None)
            block_output = result_tuple[0]
            logger.info(f"  Block {i+1} Output Shape: {block_output.shape}")
            logger.info(f"  Block {i+1} Output (first 5 tokens, first 5 dimensions):\n{block_output[0, :5, :5]}")
            current_output = block_output # Update current_output for the next block
        except Exception as e:
            logger.error(f"Error during inspection of Transformer Block {i+1}: {e}")
            return # Exit if a block fails

    # 4. Output Projection Layer
    logger.info("\nStep 4: Output Projection Layer")
    try:
        # Apply the final output projection
        # Accessing output_proj through the nested model object
        logits = current_output @ model.model.output_proj.value
        logger.info(f"Logits shape: {logits.shape}")
        logger.info(f"Logits (first 5 tokens, first 5 vocab dims):\n{logits[0, :5, :5]}")
    except Exception as e:
        logger.error(f"Error during output projection: {e}")

    logger.info("\n--- Forward Pass Inspection Complete ---")

def inspect_model_parameters_detailed(model):
    """
    Provides a more detailed inspection of all model parameters.
    """
    logger.info("\n--- Detailed Model Parameter Inspection ---")
    if hasattr(model, 'parameters') and callable(model.parameters):
        params = model.parameters() # This returns a list of Parameter objects
        
        # Iterate over the list of Parameter objects
        for param in params:
            # Assuming each Parameter object has 'name' and 'value' attributes
            name = param.name
            value = param.value
            
            logger.info(f"Parameter: {name}")
            logger.info(f"  Shape: {value.shape}")
            logger.info(f"  Mean: {np.mean(value):.6f}")
            logger.info(f"  Std Dev: {np.std(value):.6f}")
            logger.info(f"  Min: {np.min(value):.6f}")
            logger.info(f"  Max: {np.max(value):.6f}")
            logger.info(f"  NaNs: {np.sum(np.isnan(value))}")
            logger.info(f"  Infs: {np.sum(np.isinf(value))}")
            # Add more checks if needed, e.g., distribution plots
    else:
        logger.warning("Model does not have a 'parameters' method for detailed inspection.")


# Example usage (requires model, tokenizer, and model_config to be loaded)
# if __name__ == "__main__":
#     # Dummy classes for demonstration if actual ones are not available
#     class DummyTokenizer:
#         def encode(self, text): return [ord(c) for c in text]
#         def decode(self, ids): return "".join([chr(i) for i in ids])
#         def get_vocab_size(self): return 256
# 
#     class DummyModel:
#         def __init__(self, config): self.config = config
#         def get_embeddings(self, input_tensor): return np.random.rand(input_tensor.shape[0], input_tensor.shape[1], self.config['n_embd'])
#         def forward_with_intermediates(self, input_tensor):
#             # Simulate passing through a few blocks
#             output = self.get_embeddings(input_tensor)
#             intermediates = []
#             for _ in range(self.config['n_layer']):
#                 output = output + np.random.rand(*output.shape) * 0.1 # Simulate some transformation
#                 intermediates.append(output)
#             return intermediates
#         def forward(self, input_tensor):
#             output = self.forward_with_intermediates(input_tensor)[-1]
#             return output @ np.random.rand(self.config['n_embd'], self.config['vocab_size'])
#         def parameters(self):
#             return {
#                 "w_embed": np.random.rand(self.config['vocab_size'], self.config['n_embd']),
#                 "transformer.0.attn.w_q": np.random.rand(self.config['n_embd'], self.config['n_embd']),
#             }
# 
#     dummy_model_config = {
#         "vocab_size": 256, "n_layer": 2, "n_head": 4, "n_embd": 64, "block_size": 10
#     }
# 
#     tokenizer = DummyTokenizer()
#     model = DummyModel(dummy_model_config)
# 
#     inspect_model_forward_pass(model, tokenizer, "hello world", dummy_model_config["block_size"])
#     inspect_model_parameters_detailed(model)
