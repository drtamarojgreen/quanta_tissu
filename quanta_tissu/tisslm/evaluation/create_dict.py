
import os
import sys
import argparse
import json
import re
from collections import Counter

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

OUTPUT_FILENAME = os.path.join(project_root, "data", "corpus_dict.json")
WORD_COUNT = 5000

def create_dictionary_from_corpus(corpus_dir):
    """
    Scans all text files in a directory, counts word frequencies,
    and returns a list of the most common words.
    """
    word_counts = Counter()
    word_pattern = re.compile(r'\b\w+\b')

    print(f"Scanning files in '{corpus_dir}'...")
    file_count = 0
    for root, _, files in os.walk(corpus_dir):
        for file in files:
            # You can add more extensions if needed, e.g., .md, .rst
            if file.endswith(".txt"):
                file_path = os.path.join(root, file)
                file_count += 1
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        text = f.read().lower()
                        words = word_pattern.findall(text)
                        word_counts.update(words)
                except Exception as e:
                    print(f"Warning: Could not process file {file_path}: {e}")
    
    print(f"Scanned {file_count} files.")
    print(f"Found {len(word_counts)} unique words.")

    # Get the most common words
    most_common_words = [word for word, count in word_counts.most_common(WORD_COUNT)]
    
    return most_common_words

def save_dictionary_to_json(word_list, output_path):
    """
    Saves a list of words to a JSON file.
    """
    print(f"Saving {len(word_list)} most common words to {output_path}...")
    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(word_list, f, indent=2)
        print("Dictionary saved successfully.")
    except IOError as e:
        print(f"Error: Could not write to file {output_path}: {e}")

def main():
    """Main function to run the dictionary creation process."""
    parser = argparse.ArgumentParser(
        description=f"Create a dictionary of the {WORD_COUNT} most common words from a corpus."
    )
    parser.add_argument(
        "corpus_dir",
        type=str,
        help="The directory containing the corpus files to scan."
    )
    args = parser.parse_args()

    if not os.path.isdir(args.corpus_dir):
        print(f"Error: Corpus directory not found at '{args.corpus_dir}'")
        return

    most_common_words = create_dictionary_from_corpus(args.corpus_dir)
    
    if most_common_words:
        save_dictionary_to_json(most_common_words, OUTPUT_FILENAME)
    else:
        print("No words found to create a dictionary.")

if __name__ == "__main__":
    main()
