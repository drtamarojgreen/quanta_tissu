import numpy as np
import os
import logging
import json # Added import

from .architecture.llm import Model
from .generation.generator import Generator
from .generation.alg_generator import AlgorithmicGenerator
from .generation.config import GenerationConfig # Added import
from .knowledge_base import KnowledgeBase
from .tokenizer import Tokenizer
from .embedding.embedder import Embedder
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

        # Load paths from configuration file
        project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
        config_path = os.path.join(project_root, 'quanta_tissu', 'configurations', 'paths.json')
        try:
            with open(config_path, 'r') as f:
                paths_config = json.load(f)
        except FileNotFoundError:
            logger.error(f"Configuration file not found at {config_path}")
            raise

        # Instantiate the tokenizer
        tokenizer_dir = os.path.join(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')), paths_config.get("tokenizer_dir"))
        tokenizer_filename_prefix = paths_config.get("tokenizer_filename_prefix")
        full_tokenizer_prefix = os.path.join(tokenizer_dir, tokenizer_filename_prefix)
        self.tokenizer = Tokenizer(tokenizer_prefix=full_tokenizer_prefix)

        # Instantiate the core model architecture
        self.model = Model(config)

        # Instantiate the generator
        self.generator = Generator(self.model, self.tokenizer)
        self.alg_generator = AlgorithmicGenerator(self.model, self.config)

        self.knowledge_base = None
        if use_db:
            try:
                logger.info(f"Initializing KnowledgeBase with TissDB connection to {db_host}:{db_port}")
                self.embedder = Embedder(self.tokenizer, self.model.embeddings.value) # Instantiate the embedder
                self.knowledge_base = KnowledgeBase(self.embedder, db_client=TissDBClient(db_host=db_host, db_port=db_port))
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

    @property
    def embeddings(self):
        """
        Returns the embeddings layer of the underlying model.
        """
        return self.model.embeddings

    def forward(self, token_ids, kv_cache=None, start_pos=0):
        """
        Performs a forward pass through the underlying model.
        Delegates to the internal model instance.
        """
        return self.model.forward(token_ids, kv_cache, start_pos)

    def backward(self, d_logits, model_cache):
        """
        Performs the backward pass through the underlying model.
        Delegates to the internal model instance.
        """
        return self.model.backward(d_logits, model_cache)

    def sample(self, prompt_token_ids, n_new_tokens, **kwargs):
        """
        Delegates to the internal generator to sample new tokens.
        """
        # Create a GenerationConfig object from kwargs
        config = GenerationConfig(
            method=kwargs.get('method', 'greedy'),
            temperature=kwargs.get('temperature', 1.0),
            top_k=kwargs.get('top_k'),
            top_p=kwargs.get('top_p'),
            repetition_penalty=kwargs.get('repetition_penalty', 1.0),
            bias_token_id=kwargs.get('bias_token_id'),
            bias_strength=kwargs.get('bias_strength'),
            eos_id=kwargs.get('eos_id'),
            suppress_eos=kwargs.get('suppress_eos', False),
            no_repeat_ngram_size=kwargs.get('no_repeat_ngram_size', 0),
            logit_bias=kwargs.get('logit_bias')
        )
        generated_output, _ = self.generator.sample([prompt_token_ids], n_new_tokens, config, **kwargs)
        if not generated_output:
            return []
        return generated_output[0]
