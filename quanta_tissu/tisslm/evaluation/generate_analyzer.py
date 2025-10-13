import argparse
import re
import os

def analyze_generation_output(file_path):
    """
    Reads the output file, extracts method names and generated text.
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: Input file not found at {file_path}")
        return
    except UnicodeDecodeError:
        print(f"Warning: UTF-8 decoding failed for {file_path}. Attempting with latin-1 encoding.")
        try:
            with open(file_path, 'r', encoding='latin-1') as f:
                content = f.read()
        except Exception as e:
            print(f"Error: Could not read file with latin-1 encoding either: {e}")
            return

    # Regex to find "--- Generating text with prompt: ... using method: METHOD ---"
    # and "--- Generated Text (METHOD) ---" followed by the generated text.
    # We'll look for blocks starting with "--- Generating text..." and ending with "----------------------"
    # and extract the relevant parts.

    # This regex captures the method name and the generated text block
    # It looks for the "--- Generated Text (METHOD) ---" line and then captures everything until "----------------------"
    pattern = re.compile(r"--- Generated Text \((.*?)\) ---\s*\n(.*?)\s*\n----------------------", re.DOTALL)

    matches = pattern.finditer(content)

    found_any = False
    for match in matches:
        method = match.group(1).strip()
        generated_text = match.group(2).strip()
        print(f"--- Generated Text ({method}) ---")
        print(generated_text)
        print("----------------------")
        found_any = True
    
    if not found_any:
        print("No generated text blocks found in the file.")

def main():
    parser = argparse.ArgumentParser(description="Analyze text generation output.")
    parser.add_argument("--input_file", type=str, default="out.txt",
                        help="Path to the input file containing generation output.")
    
    args = parser.parse_args()

    # Construct the full absolute path for the input file
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust '..' count based on file location relative to project root
    input_file_full_path = os.path.join(project_root, args.input_file)

    analyze_generation_output(input_file_full_path)

if __name__ == "__main__":
    main()
