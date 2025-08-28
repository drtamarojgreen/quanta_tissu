import numpy as np
import os
import logging

from .architecture.llm import Model
from .generation.generator import Generator
from .knowledge_base import KnowledgeBase
from .tokenizer import tokenize
from .parameter import Parameter
from .model_error_handler import TissModelError, ModelProcessingError
from .system_error_handler import TissSystemError, DatabaseConnectionError

logger = logging.getLogger(__name__)

class QuantaTissu:
    """
    A high-level facade for the TissLM model.

    This class is responsible for loading the model configuration and weights,
    and providing a clean interface for users. It instantiates the core
    model architecture and delegates generation tasks to a Generator class.
    """
    def __init__(self, config, db_host='127.0.0.1', db_port=8080, use_db=False):
        self.config = config

        # Instantiate the core model architecture
        self.model = Model(config)

        # Instantiate the generator
        self.generator = Generator(self.model)

        self.knowledge_base = None
        if use_db:
            try:
                logger.info(f"Initializing KnowledgeBase with TissDB connection to {db_host}:{db_port}")
                # Note: KnowledgeBase might need refactoring if it depends on model embeddings directly
                self.knowledge_base = KnowledgeBase(self.model.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
            except DatabaseConnectionError as e:
                raise TissSystemError(f"Failed to connect to database: {e}") from e

    def parameters(self):
        """
        Returns the parameters of the underlying model.
        """
        return self.model.parameters()

    def load_weights(self, path):
        """
        Loads weights from a file and applies them to the underlying model.
        """
        if not os.path.exists(path):
            logger.warning(f"Model weights file not found at {path}. Using random initialization.")
            return

        try:
            data = np.load(path, allow_pickle=True)
            keys = list(data.keys())
            logger.info(f"Loading weights from {path}. Found keys: {keys}")

            model_params = self.model.parameters()
            param_dict = {p.name: p for p in model_params}

            is_legacy = any(k.startswith('param_') for k in keys)

            if is_legacy:
                logger.info("Detected legacy checkpoint format. Loading by parameter order.")
                param_keys = sorted([k for k in keys if k.startswith('param_')], key=lambda k: int(k.split('_')[1]))

                if len(param_keys) != len(model_params):
                    logger.warning(f"Parameter count mismatch. Checkpoint has {len(param_keys)} params, model requires {len(model_params)}.")

                for i, key in enumerate(param_keys):
                    if i < len(model_params):
                        param = model_params[i]
                        if param.value.shape == data[key].shape:
                            param.value[:] = data[key]
                        else:
                            logger.warning(f"Shape mismatch for {param.name} (from {key}). Expected {param.value.shape}, got {data[key].shape}. Skipping.")
            else:
                logger.info("Loading by hierarchical parameter name.")
                loaded_count = 0
                for name, param in param_dict.items():
                    if name in data:
                        if param.value.shape == data[name].shape:
                            param.value = data[name]
                            loaded_count += 1
                        else:
                            logger.warning(f"Shape mismatch for {name}. Expected {param.value.shape}, got {data[name].shape}. Skipping.")
                    else:
                        logger.warning(f"Parameter {name} not found in weights file. Using random initialization.")

                if loaded_count == 0 and not is_legacy:
                    logger.error("No matching parameters found by name in the checkpoint file.")
                else:
                    logger.info(f"Successfully loaded {loaded_count}/{len(model_params)} parameters by name.")

        except (IOError, FileNotFoundError) as e:
            raise TissSystemError(f"Failed to read model weights file from {path}: {e}") from e
        except Exception as e:
            logger.error(f"Error processing model weights from {path}: {e}", exc_info=True)
            raise ModelProcessingError(f"Failed to process model weights from {path}: {e}") from e

    def forward(self, token_ids, kv_cache=None, start_pos=0):
        """
        Performs a forward pass through the underlying model.
        Delegates to the internal model instance.
        """
        return self.model.forward(token_ids, kv_cache, start_pos)

    def backward(self, d_logits, cache):
        """
        Performs a backward pass through the underlying model.
        Delegates to the internal model instance.
        """
        self.model.backward(d_logits, cache)

    @property
    def embeddings(self):
        """
        Returns the embeddings layer of the underlying model.
        """
        return self.model.embeddings

    def sample(self, prompt_tokens, n_new_tokens, **kwargs):
        """
        Generates text by delegating to the Generator class.
        
        Args:
            prompt_tokens (list[int]): The initial sequence of token IDs.
            n_new_tokens (int): The number of new tokens to generate.
            **kwargs: Additional arguments for the generator, e.g., method, temperature.
            
        Returns:
            list[int]: The list of newly generated token IDs.
        """
        return self.generator.sample(prompt_tokens, n_new_tokens, **kwargs)