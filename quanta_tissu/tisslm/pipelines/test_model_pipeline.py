import sys
from pathlib import Path
import pytest

# --- Path Setup ---
# By adding the parent 'tisslm' directory to the system path, we can directly
# import modules from the 'core' directory. This is a more standard and robust
# approach than using importlib.
tisslm_dir = Path(__file__).resolve().parent.parent
core_dir = tisslm_dir / "core"
sys.path.insert(0, str(tisslm_dir))

from core import train_bpe, run_training, generate_text

# --- Pytest Fixtures ---
# Fixtures are a powerful pytest feature for setting up test environments.
# They provide a clean, reusable way to manage resources like paths and data.

@pytest.fixture(scope="module")
def corpus_path() -> Path:
    """Provides the path to the corpus directory."""
    path = tisslm_dir / "corpus"
    assert path.exists(), "Corpus directory not found. Tests cannot run."
    return path

@pytest.fixture(scope="module")
def trained_tokenizer_prefix(tmp_path_factory, corpus_path) -> Path:
    """
    Trains the BPE tokenizer once per test session and returns the path prefix.
    This uses a temporary directory to avoid cluttering the project.
    `scope="module"` ensures this expensive operation runs only once for all tests.
    """
    tokenizer_dir = tmp_path_factory.mktemp("tokenizer_assets")
    prefix = tokenizer_dir / "trained_tokenizer"

    print(f"\n(Setup) Invoking BPE Tokenizer Training...")
    try:
        # We assume train_bpe.main is compatible with Path objects
        train_bpe.main(corpus_path=str(corpus_path), save_prefix=str(prefix))
        print("(Setup) BPE Training completed.")
    except Exception as e:
        pytest.fail(f"BPE Tokenizer training failed during setup: {e}")

    # Assertion: Verify that the tokenizer model file was created.
    model_file = prefix.with_suffix(".model")
    assert model_file.exists(), "Tokenizer model file was not created."

    return prefix

# --- Test Functions ---

@pytest.mark.parametrize("run_number", range(1, 4))
def test_training_and_generation_cycle(run_number, corpus_path, trained_tokenizer_prefix, tmp_path):
    """
    Tests a full training and generation cycle.
    This test is parameterized to run 3 times, simulating the original script's loop.
    `tmp_path` is a pytest fixture that provides a unique temporary directory for each test run.
    """
    print(f"\n--- [Run {run_number}/3] ---")
    checkpoint_dir = tmp_path / "checkpoints"
    checkpoint_dir.mkdir()

    # --- 1. Model Training ---
    print(f"[Run {run_number}] Invoking Model Training...")
    try:
        # NOTE: For this to be fully testable, `run_training.main` and
        # `generate_text.main` should ideally accept the tokenizer_prefix path
        # as an argument instead of relying on a hardcoded path.
        run_training.main(
            corpus_path=str(corpus_path),
            checkpoint_dir=str(checkpoint_dir),
            epochs=1,
            save_every=1
        )
        print(f"[Run {run_number}] Model Training completed.")
    except Exception as e:
        pytest.fail(f"[Run {run_number}] Model Training failed: {e}")

    # Assertion: Check that a checkpoint file was created.
    checkpoint_files = list(checkpoint_dir.glob("*.npz"))
    assert len(checkpoint_files) > 0, "No checkpoint file (.npz) was created."
    checkpoint_path = checkpoint_files[0]

    # --- 2. Text Generation ---
    prompt = "Do you know this is a test?"
    print(f"[Run {run_number}] Invoking Text Generation...")
    try:
        output_text = generate_text.main(
            prompt=prompt,
            checkpoint_path=str(checkpoint_path),
            length=50
        )
        print(f"[Run {run_number}] Generated text: {output_text}")
    except Exception as e:
        pytest.fail(f"[Run {run_number}] Text Generation failed: {e}")

    # Assertions: Verify the generated text is valid.
    assert isinstance(output_text, str)
    assert len(output_text) > len(prompt)
    assert output_text.startswith(prompt)

