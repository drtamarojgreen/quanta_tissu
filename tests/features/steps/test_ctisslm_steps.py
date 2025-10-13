import os

def register_steps(runner):

    def reset_context(context):
        # We'll use a more complex state to simulate file systems and loaded models
        context.sim_state = {
            "file_system": set(),
            "ctisslm_instance": None,
            "model_loaded": False,
            "tokenizer_loaded": False,
            "last_generated_text": None,
            "last_token_ids": None
        }

    @runner.step(r'a valid model file exists at "(.*)"')
    def valid_model_exists(context, path):
        reset_context(context)
        context.sim_state["file_system"].add(path)
        print(f"SIM: Virtual file created at '{path}'.")

    @runner.step(r'a valid tokenizer file exists at "(.*)"')
    def valid_tokenizer_exists(context, path):
        # Can be called after the first step, so don't reset
        context.sim_state["file_system"].add(path)
        print(f"SIM: Virtual file created at '{path}'.")

    @runner.step(r'no model file exists at "(.*)"')
    def no_model_exists(context, path):
        reset_context(context)
        if path in context.sim_state["file_system"]:
            context.sim_state["file_system"].remove(path)
        print(f"SIM: Ensured no file exists at '{path}'.")

    @runner.step(r'an instance of ctisslm is created')
    def create_instance(context):
        context.sim_state["ctisslm_instance"] = {"status": "created"}
        print("SIM: ctisslm instance created.")

    @runner.step(r'the model is loaded from "(.*)"')
    def load_model(context, path):
        if path in context.sim_state["file_system"]:
            context.sim_state["model_loaded"] = True
            print(f"SIM: Model loaded successfully from '{path}'.")
        else:
            raise FileNotFoundError(f"Model file not found at {path}")

    @runner.step(r'the tokenizer is loaded from "(.*)"')
    def load_tokenizer(context, path):
        if path in context.sim_state["file_system"]:
            context.sim_state["tokenizer_loaded"] = True
            print(f"SIM: Tokenizer loaded successfully from '{path}'.")
        else:
            raise FileNotFoundError(f"Tokenizer file not found at {path}")

    @runner.step(r'the model and tokenizer should be loaded successfully')
    def model_and_tokenizer_loaded(context):
        assert context.sim_state["model_loaded"]
        assert context.sim_state["tokenizer_loaded"]
        print("SIM: Verified model and tokenizer are loaded.")

    @runner.step(r'the application attempts to load the model from "(.*)"')
    def attempt_load_model(context, path):
        try:
            load_model(context, path)
            context.sim_state["load_failed"] = False
        except FileNotFoundError:
            context.sim_state["load_failed"] = True
        print(f"SIM: Application attempted to load model from '{path}'.")

    @runner.step(r'the model loading should fail')
    def model_loading_fails(context):
        assert context.sim_state["load_failed"]
        print("SIM: Verified model loading failed as expected.")

    @runner.step(r'a ctisslm instance with a loaded model and tokenizer')
    def instance_with_model_and_tokenizer(context):
        reset_context(context)
        context.sim_state["file_system"].add("models/ctisslm.bin")
        context.sim_state["file_system"].add("models/tokenizer.json")
        create_instance(context)
        load_model(context, "models/ctisslm.bin")
        load_tokenizer(context, "models/tokenizer.json")
        model_and_tokenizer_loaded(context)
        print("SIM: A fully loaded ctisslm instance is ready.")

    @runner.step(r'the application generates text from the prompt "(.*)"')
    def generate_text(context, prompt):
        # Simple simulation of text generation
        context.sim_state["last_generated_text"] = f"{prompt} ... and so on."
        print(f"SIM: Generated text for prompt '{prompt}'.")

    @runner.step(r'it should return a non-empty generated string')
    def returns_non_empty_string(context):
        assert context.sim_state["last_generated_text"]
        print("SIM: Verified generated text is not empty.")

    @runner.step(r'the application generates text from the prompt "(.*)" with a max_length of (\d+)')
    def generate_with_max_length(context, prompt, max_length):
        max_length = int(max_length)
        # Simulate generation with token limit
        generated_tokens = ["token"] * max_length
        context.sim_state["last_generated_text"] = " ".join(generated_tokens)
        print(f"SIM: Generated text for prompt '{prompt}' with max_length {max_length}.")

    @runner.step(r'the generated text should have at most (\d+) tokens')
    def text_has_at_most_tokens(context, num_tokens):
        num_tokens = int(num_tokens)
        # Our simulation is exact, but in reality it's <=
        num_generated_tokens = len(context.sim_state["last_generated_text"].split())
        assert num_generated_tokens <= num_tokens
        print(f"SIM: Verified generated text has {num_generated_tokens} tokens (<= {num_tokens}).")

    @runner.step(r'a loaded ctisslm tokenizer')
    def loaded_tokenizer(context):
        reset_context(context)
        context.sim_state["tokenizer_loaded"] = True
        print("SIM: A loaded tokenizer is ready.")

    @runner.step(r'the text "(.*)" is encoded')
    def text_is_encoded(context, text):
        # Simulate encoding: text -> list of ints
        context.sim_state["last_token_ids"] = [ord(c) for c in text]
        print(f"SIM: Encoded '{text}' to {context.sim_state['last_token_ids']}.")

    @runner.step(r'it should produce a sequence of token IDs')
    def produces_token_ids(context):
        assert context.sim_state["last_token_ids"] is not None
        assert all(isinstance(i, int) for i in context.sim_state["last_token_ids"])
        print("SIM: Verified a sequence of token IDs was produced.")

    @runner.step(r'the token IDs are decoded')
    def ids_are_decoded(context):
        # Simulate decoding: list of ints -> text
        context.sim_state["last_generated_text"] = "".join([chr(i) for i in context.sim_state["last_token_ids"]])
        print(f"SIM: Decoded {context.sim_state['last_token_ids']} to '{context.sim_state['last_generated_text']}'.")

    @runner.step(r'the result should be the original text "(.*)"')
    def result_is_original_text(context, text):
        assert context.sim_state["last_generated_text"] == text
        print(f"SIM: Verified decoded text matches original '{text}'.")
