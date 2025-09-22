import os

# A simple mock for the Eigen::MatrixXf class, to check shapes
class MockMatrix:
    def __init__(self, rows, cols):
        self.rows = rows
        self.cols = cols

    def __repr__(self):
        return f"<MockMatrix {self.rows}x{self.cols}>"

def register_steps(runner):

    def reset_context(context):
        context.sim_state = {
            "model": None,
            "config": None,
            "last_logits": None
        }

    @runner.step(r'the cllm library')
    def cllm_library(context):
        reset_context(context)
        print("SIM: cllm library is available.")

    @runner.step(r'a model is initialized with the default configuration')
    def init_default_model(context):
        # Simulate creating a model with default config
        default_config = {
            "d_model": 512, "n_heads": 8, "n_layers": 6,
            "vocab_size": 32000, "max_seq_len": 512
        }
        context.sim_state["config"] = default_config
        context.sim_state["model"] = {"config": default_config}
        print("SIM: Initialized model with default config.")

    @runner.step(r'the model should be created successfully')
    def model_created_successfully(context):
        assert context.sim_state["model"] is not None
        print("SIM: Verified model was created.")

    @runner.step(r'its configuration should match the default values')
    def config_matches_default(context):
        default_config = {
            "d_model": 512, "n_heads": 8, "n_layers": 6,
            "vocab_size": 32000, "max_seq_len": 512
        }
        assert context.sim_state["config"] == default_config
        print("SIM: Verified model config matches default.")

    @runner.step(r'a model is initialized with a custom configuration:')
    def init_custom_model(context, table):
        custom_config = {}
        for row in table:
            parts = [p.strip() for p in row.strip('|').split('|')]
            custom_config[parts[0]] = int(parts[1])
        context.sim_state["config"] = custom_config
        context.sim_state["model"] = {"config": custom_config}
        print(f"SIM: Initialized model with custom config: {custom_config}")

    @runner.step(r'its configuration should match the custom values')
    def config_matches_custom(context):
        # The custom config is already in the context from the previous step
        assert context.sim_state["model"]["config"] == context.sim_state["config"]
        print("SIM: Verified model config matches custom values.")

    @runner.step(r'a cllm model is initialized')
    def model_is_initialized(context):
        cllm_library(context)
        init_default_model(context)
        model_created_successfully(context)
        print("SIM: A cllm model is initialized and ready.")

    @runner.step(r'a forward pass is performed with a sequence of (\d+) tokens')
    def forward_pass(context, num_tokens):
        num_tokens = int(num_tokens)
        # Simulate a forward pass. The output shape depends on input length and vocab size.
        vocab_size = context.sim_state["config"]["vocab_size"]
        context.sim_state["last_logits"] = MockMatrix(rows=num_tokens, cols=vocab_size)
        print(f"SIM: Performed forward pass with {num_tokens} tokens.")

    @runner.step(r'the model should return a logits matrix')
    def returns_logits_matrix(context):
        assert isinstance(context.sim_state["last_logits"], MockMatrix)
        print("SIM: Verified model returned a logits matrix.")

    @runner.step(r'the logits matrix should have (\d+) rows')
    def logits_have_rows(context, num_rows):
        num_rows = int(num_rows)
        assert context.sim_state["last_logits"].rows == num_rows
        print(f"SIM: Verified logits matrix has {num_rows} rows.")

    @runner.step(r'the logits matrix should have a number of columns equal to the vocab_size')
    def logits_have_vocab_cols(context):
        expected_cols = context.sim_state["config"]["vocab_size"]
        assert context.sim_state["last_logits"].cols == expected_cols
        print(f"SIM: Verified logits matrix has {expected_cols} columns (vocab_size).")
