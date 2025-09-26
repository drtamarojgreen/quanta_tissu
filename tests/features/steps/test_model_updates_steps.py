import numpy as np

# Mocking the QuantaTissu model for the purpose of these BDD tests
# In a real test, this might import the actual model
class MockQuantaTissu:
    def __init__(self, config):
        self.config = config
        self.vocab_size = config.get('vocab_size', 100)

    def forward(self, token_ids):
        batch_size, seq_len = token_ids.shape
        # Return a mock logits matrix of the correct shape
        return np.random.rand(batch_size, seq_len, self.vocab_size), None

def register_steps(runner):

    @runner.step(r'a model is configured to use the "([^"]*)" mechanism')
    def configure_attention_mechanism(context, mechanism_name):
        if not hasattr(context, 'config'):
            context.config = {}
        context.config['attention_mechanism'] = mechanism_name
        print(f"SIM: Configured attention mechanism to '{mechanism_name}'.")

    @runner.step(r'the model is initialized')
    def initialize_model(context):
        # Use a default config if none is provided
        if not hasattr(context, 'config'):
            context.config = {
                'vocab_size': 100, 'n_embd': 64, 'n_layer': 2, 'n_head': 2,
                'd_ff': 128, 'max_seq_len': 256, 'dropout': 0.1
            }
        context.model = MockQuantaTissu(context.config)
        assert context.model is not None
        print("SIM: Model initialized.")

    @runner.step(r'the model should be configured to use the new attention mechanism')
    def verify_attention_mechanism(context):
        assert context.model.config.get('attention_mechanism') == 'new_attention'
        print("SIM: Verified model is configured for 'new_attention'.")

    @runner.step(r'a model is configured with the "([^"]*)" activation function')
    def configure_activation_function(context, activation_function):
        if not hasattr(context, 'config'):
            context.config = {}
        context.config['activation_function'] = activation_function
        print(f"SIM: Configured activation function to '{activation_function}'.")

    @runner.step(r'a forward pass is performed with a sequence of (\d+) tokens')
    def perform_forward_pass(context, num_tokens):
        num_tokens = int(num_tokens)
        token_ids = np.random.randint(0, 100, size=(1, num_tokens))
        context.logits, _ = context.model.forward(token_ids)
        print(f"SIM: Performed forward pass with {num_tokens} tokens.")

    @runner.step(r'the model should return a valid logits matrix for the new activation function')
    def verify_logits_matrix(context):
        assert context.logits is not None
        assert isinstance(context.logits, np.ndarray)
        # In a real test, we might check for specific properties of the output
        # that are characteristic of the new activation function.
        print("SIM: Verified a valid logits matrix was returned.")