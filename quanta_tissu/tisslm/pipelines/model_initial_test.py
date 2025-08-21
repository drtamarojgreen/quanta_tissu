import subprocess
import os
import sys

# Define paths relative to the script's location
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Go up from pipelines/quanta_tissu/quanta_tissu/tisslm to quanta_tissu/

tisslm_module_prefix = "quanta_tissu.tisslm.core"
models_dir = os.path.join(project_root, "models")
corpus_dir = os.path.join(project_root, "corpus")
output_file = os.path.join(script_dir, "test_output.txt")

tokenizer_prefix = os.path.join(models_dir, "trained_tokenizer")
checkpoint_dir = os.path.join(models_dir, "checkpoints")
checkpoint_path = os.path.join(checkpoint_dir, "checkpoint_test_001.npz")

# Ensure checkpoints directory exists
os.makedirs(checkpoint_dir, exist_ok=True)

# Clear previous output file
if os.path.exists(output_file):
    os.remove(output_file)

def run_command(cmd, log_message, error_message):
    with open(output_file, "a") as f:
        f.write(f"{log_message}\n")
        print(log_message) # Also print to console for immediate feedback
        
        # Debugging: Print the command being executed
        print(f"DEBUG: Executing command: {' '.join(cmd)}")
        f.write(f"DEBUG: Executing command: {' '.join(cmd)}\n")

        try:
            # Use subprocess.run to execute the command
            # capture_output=True captures stdout and stderr
            # text=True decodes stdout/stderr as text
            # cwd=project_root ensures Python finds the 'quanta_tissu' package
            # Added shell=True for Windows compatibility with paths
            result = subprocess.run(cmd, capture_output=True, text=True, check=True, cwd=project_root, shell=True)
            f.write(result.stdout)
            f.write(result.stderr)
            f.write(f"{error_message.replace('failed', 'completed')}\n") # Log completion message
            print(error_message.replace('failed', 'completed'))
        except subprocess.CalledProcessError as e:
            f.write(e.stdout)
            f.write(e.stderr)
            f.write(f"{error_message}: {e}\n")
            print(f"{error_message}: {e}")
            sys.exit(1) # Exit if a command fails
        except Exception as e:
            f.write(f"An unexpected error occurred: {e}\n")
            print(f"An unexpected error occurred: {e}")
            sys.exit(1)

# Loop 3 times
for i in range(1, 4):
    log_header = f"\n--- Starting Run {i}/3 ---"
    run_command([], log_header, "") # Just log the header

    # 1. Train BPE tokens
    log_msg = f"[Run {i}] Invoking BPE Tokenizer Training..."
    err_msg = f"[Run {i}] BPE Training failed."
    # Construct command as a single string for shell=True
    cmd_str = f"{sys.executable} -u -m {tisslm_module_prefix}.train_bpe --corpus_path \"{corpus_dir}\" --save_prefix \"{tokenizer_prefix}\"".replace('\\', '/') # Replace backslashes with forward slashes for shell
    run_command(cmd_str, log_msg, err_msg)

    # 2. Create a checkpoint named checkpoint_test_001
    log_msg = f"[Run {i}] Invoking Model Training..."
    err_msg = f"[Run {i}] Model Training failed."
    # Ensure the checkpoint is clean for each run
    if os.path.exists(checkpoint_path):
        os.remove(checkpoint_path)
    # Construct command as a single string for shell=True
    cmd_str = f"{sys.executable} -u -m {tisslm_module_prefix}.run_training --corpus_path \"{corpus_dir}\" --checkpoint_dir \"{checkpoint_dir}\" --epochs 1 --save_every 1".replace('\\', '/') # Replace backslashes with forward slashes for shell
    run_command(cmd_str, log_msg, err_msg)

    # 3. Use generate_text script to test a prompt
    prompt = "Do you know this is a test?"
    log_msg = f"[Run {i}] Invoking Text Generation..."
    err_msg = f"[Run {i}] Text Generation failed."
    # Construct command as a single string for shell=True
    cmd_str = f"{sys.executable} -u -m {tisslm_module_prefix}.generate_text --prompt \"{prompt}\" --checkpoint_path \"{checkpoint_path}\" --length 50".replace('\\', '/') # Replace backslashes with forward slashes for shell
    run_command(cmd_str, log_msg, err_msg)

    log_footer = f"--- Finished Run {i}/3 ---"
    run_command([], log_footer, "") # Just log the footer

final_message = "\nAll runs complete. Check test_output.txt for details."
run_command([], final_message, "") # Just log the final message