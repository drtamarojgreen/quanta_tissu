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
=======
class TestModelPipeline(unittest.TestCase):
    _temp_dir = None
    _trained_tokenizer_prefix = None
    _corpus_path = None

    @classmethod
    def setUpClass(cls):
        """
        Set up resources that are shared across all tests in this class.
        This runs once before any tests.
        """
        # Create a temporary directory for all test artifacts
        cls._temp_dir = tempfile.mkdtemp()

        # --- Setup Corpus Path ---
        cls._corpus_path = tisslm_dir / "corpus"
        if not cls._corpus_path.exists():
            raise FileNotFoundError("Corpus directory not found. Tests cannot run.")

        # --- Train Tokenizer ---
        tokenizer_dir = Path(cls._temp_dir) / "tokenizer_assets"
        tokenizer_dir.mkdir()
        cls._trained_tokenizer_prefix = tokenizer_dir / "trained_tokenizer"

        print(f"\n(Setup) Invoking BPE Tokenizer Training...")
        try:
            train_bpe.main(corpus_path=str(cls._corpus_path), save_prefix=str(cls._trained_tokenizer_prefix))
            print("(Setup) BPE Training completed.")
        except Exception as e:
            # Can't use self.fail in a class method, so raise an exception
            raise Exception(f"BPE Tokenizer training failed during setup: {e}")

        # Verify that the tokenizer model file was created.
        model_file = cls._trained_tokenizer_prefix.with_suffix(".model")
        if not model_file.exists():
            raise FileNotFoundError("Tokenizer model file was not created.")

    @classmethod
    def tearDownClass(cls):
        """
        Clean up resources after all tests in this class have run.
        This runs once after all tests.
        """
        if cls._temp_dir:
            shutil.rmtree(cls._temp_dir)
            print(f"\n(Teardown) Removed temporary directory: {cls._temp_dir}")

    def test_training_and_generation_cycle(self):
        """
        Tests a full training and generation cycle.
        This test is looped to run 3 times, simulating the original parameterized test.
        """
        for run_number in range(1, 4):
            with self.subTest(run=run_number):
                print(f"\n--- [Run {run_number}/3] ---")

                # Each test run gets its own subdirectory for checkpoints
                checkpoint_dir = Path(self._temp_dir) / f"run_{run_number}_checkpoints"
                checkpoint_dir.mkdir()

                # --- 1. Model Training ---
                print(f"[Run {run_number}] Invoking Model Training...")
                try:
                    run_training.main(
                        corpus_path=str(self._corpus_path),
                        checkpoint_dir=str(checkpoint_dir),
                        epochs=1,
                        save_every=1
                    )
                    print(f"[Run {run_number}] Model Training completed.")
                except Exception as e:
                    self.fail(f"[Run {run_number}] Model Training failed: {e}")

                # Assertion: Check that a checkpoint file was created.
                checkpoint_files = list(checkpoint_dir.glob("*.npz"))
                self.assertGreater(len(checkpoint_files), 0, "No checkpoint file (.npz) was created.")
                checkpoint_path = checkpoint_files[0]

                # --- 2. Text Generation ---
                prompt = "Do you know this is a test?"
                print(f"[Run {run_number}] Invoking Text Generation...")
                try:
                    # NOTE: This assumes that generate_text.main knows where to find the
                    # tokenizer model. If it relies on a hardcoded path, this test might
                    # need adjustment to pass the tokenizer path explicitly.
                    output_text = generate_text.main(
                        prompt=prompt,
                        checkpoint_path=str(checkpoint_path),
                        length=50
                    )
                    print(f"[Run {run_number}] Generated text: {output_text}")
                except Exception as e:
                    self.fail(f"[Run {run_number}] Text Generation failed: {e}")

                # Assertions: Verify the generated text is valid.
                self.assertIsInstance(output_text, str)
                self.assertGreater(len(output_text), len(prompt))
                self.assertTrue(output_text.startswith(prompt))

if __name__ == '__main__':
    unittest.main()
