import sys
import os
from pathlib import Path

# --- Path Setup ---
tisslm_dir = Path(__file__).resolve().parent.parent
core_dir = tisslm_dir / "core"
sys.path.insert(0, str(tisslm_dir))

try:
    from core import train_bpe, run_training, generate_text
except ImportError as e:
    print(f"Error importing core modules: {e}")
    sys.exit(1)

# --- Manual File Utilities (replacing shutil) ---

def _rmtree(top: Path):
    """
    Manually implements shutil.rmtree using os module.
    """
    if not top.is_dir():
        return
    # Use os.walk to traverse from the bottom up
    for root, dirs, files in os.walk(str(top), topdown=False):
        for name in files:
            os.remove(os.path.join(root, name))
        for name in dirs:
            os.rmdir(os.path.join(root, name))
    os.rmdir(str(top))

# --- Test Helper Functions ---

def _get_corpus_path() -> Path:
    """Gets the path to the corpus directory and ensures it exists."""
    path = tisslm_dir / "corpus"
    if not path.exists():
        raise FileNotFoundError("Corpus directory not found. Tests cannot run.")
    return path

def _train_tokenizer(temp_dir: Path, corpus_path: Path) -> Path:
    """
    Trains the BPE tokenizer in a temporary directory and returns the path prefix.
    """
    tokenizer_dir = temp_dir / "tokenizer_assets"
    tokenizer_dir.mkdir()
    prefix = tokenizer_dir / "trained_tokenizer"

    print(f"\n(Setup) Invoking BPE Tokenizer Training...")
    try:
        train_bpe.main(corpus_path=str(corpus_path), save_prefix=str(prefix))
        print("(Setup) BPE Training completed.")
    except Exception as e:
        raise AssertionError(f"BPE Tokenizer training failed during setup: {e}")

    model_file = prefix.with_suffix(".model")
    if not model_file.exists():
        raise AssertionError("Tokenizer model file was not created.")
    return prefix

# --- Main Test Function ---

def test_training_and_generation_cycle():
    """
    Tests a full training and generation cycle.
    Manages its own setup and teardown using a manual temporary directory.
    """
    # Manually define and manage a temporary directory
    temp_dir = Path("./temp_pipeline_test")

    # Ensure cleanup before starting, in case of a previous crash
    _rmtree(temp_dir)
    temp_dir.mkdir(exist_ok=True)

    try:
        # --- Setup Phase ---
        try:
            corpus_path = _get_corpus_path()
            trained_tokenizer_prefix = _train_tokenizer(temp_dir, corpus_path)
        except (FileNotFoundError, AssertionError) as e:
            raise AssertionError(f"Test setup failed: {e}") from e

        # --- Test Execution Phase ---
        for run_number in range(1, 4):
            print(f"\n--- [Run {run_number}/3] ---")

            run_artifacts_dir = temp_dir / f"run_{run_number}"
            run_artifacts_dir.mkdir()
            checkpoint_dir = run_artifacts_dir / "checkpoints"
            checkpoint_dir.mkdir()

            # --- 1. Model Training ---
            print(f"[Run {run_number}] Invoking Model Training...")
            try:
                run_training.main(
                    corpus_path=str(corpus_path),
                    checkpoint_dir=str(checkpoint_dir),
                    epochs=1,
                    save_every=1
                )
                print(f"[Run {run_number}] Model Training completed.")
            except Exception as e:
                raise AssertionError(f"[Run {run_number}] Model Training failed: {e}")

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
                raise AssertionError(f"[Run {run_number}] Text Generation failed: {e}")

            assert isinstance(output_text, str), f"Generated output is not a string: {type(output_text)}"
            assert len(output_text) > len(prompt), "Generated text is not longer than the prompt."
            assert output_text.startswith(prompt), "Generated text does not start with the prompt."

        print("\nTest cycle completed successfully for all runs.")

    finally:
        # --- Teardown Phase ---
        print("\n(Teardown) Cleaning up temporary directory...")
        _rmtree(temp_dir)
        print("(Teardown) Cleanup complete.")

# To allow direct execution of this file for debugging
if __name__ == "__main__":
    print("Running model pipeline test directly...")
    try:
        test_training_and_generation_cycle()
        print("\n✅ Test passed.")
    except AssertionError as e:
        print(f"\n❌ Test FAILED: {e}")
    except Exception as e:
        print(f"\n❌ An unexpected error occurred: {e}")
