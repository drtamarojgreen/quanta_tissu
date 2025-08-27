import sys
import unittest
from pathlib import Path
import tempfile
import shutil

# --- Path Setup ---
tisslm_dir = Path(__file__).resolve().parent.parent
core_dir = tisslm_dir / "core"
sys.path.insert(0, str(tisslm_dir))

from core import train_bpe, run_training, generate_text

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
