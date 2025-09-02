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

from quanta_tissu.tisslm.evaluation.generate_similar import levenshtein_distance # Reusing Levenshtein

# --- Configuration ---
CORPUS_DIR_DEFAULT = os.path.join(project_root, "corpus")
SENTIMENTS_CAT_PATH_DEFAULT = os.path.join(project_root, "data", "sentiments.cat")
OUTPUT_PATH_DEFAULT = os.path.join(project_root, "data", "corpus_sentiments.cat")

def load_base_sentiment_lexicon(file_path):
    """Loads the base sentiment lexicon from a .cat file."""
    lexicon = {}
    if not os.path.exists(file_path):
        print(f"Error: Base sentiment lexicon not found at {file_path}.")
        return lexicon
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'): continue
                parts = line.split(',')
                if len(parts) == 2:
                    word = parts[0].strip().lower()
                    try:
                        score = float(parts[1].strip())
                        lexicon[word] = score
                    except ValueError:
                        print(f"Warning: Invalid score for word '{word}' in base lexicon. Skipping.")
        print(f"Loaded {len(lexicon)} words from base sentiment lexicon.")
    except IOError as e:
        print(f"Error reading base sentiment lexicon {file_path}: {e}")
    return lexicon

def scan_corpus_words(corpus_dir):
    """
    Scans all text files in a corpus directory and returns a set of unique words.
    """
    unique_words = set()
    word_pattern = re.compile(r'\b\w+\b')
    print(f"Scanning words in corpus directory '{corpus_dir}'...")
    for root, _, files in os.walk(corpus_dir):
        for file in files:
            if file.endswith(".txt"):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        text = f.read().lower()
                        words = word_pattern.findall(text)
                        unique_words.update(words)
                except Exception as e:
                    print(f"Warning: Could not process file {file_path}: {e}")
    print(f"Found {len(unique_words)} unique words in corpus.")
    return unique_words

def infer_corpus_sentiments(corpus_words, base_lexicon, max_distance_threshold):
    """
    Infers sentiment for corpus words based on the base lexicon and Levenshtein distance.
    """
    inferred_lexicon = {}
    base_lexicon_words = list(base_lexicon.keys())

    print("Inferring sentiments for corpus words...")
    for i, word in enumerate(corpus_words):
        if i % 1000 == 0: # Progress update
            print(f"  Processed {i}/{len(corpus_words)} words...")

        if word in base_lexicon:
            inferred_lexicon[word] = base_lexicon[word]
        else:
            min_distance = float('inf')
            closest_base_word = None

            for base_word in base_lexicon_words:
                distance = levenshtein_distance(word, base_word)
                if distance < min_distance:
                    min_distance = distance
                    closest_base_word = base_word
                if distance == 0: # Exact match found in base_lexicon_words (should be caught by 'word in base_lexicon' but for safety)
                    break
            
            if closest_base_word and min_distance <= max_distance_threshold:
                inferred_lexicon[word] = base_lexicon[closest_base_word]
            else:
                inferred_lexicon[word] = 0.0 # Neutral sentiment if no close match
    
    print(f"Inferred sentiments for {len(inferred_lexicon)} words.")
    return inferred_lexicon

def save_corpus_sentiments(lexicon, output_path):
    """
    Saves the inferred sentiment lexicon to a .cat file.
    """
    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            for word, score in lexicon.items():
                f.write(f"{word},{score}\n")
        print(f"Saved inferred sentiment lexicon to {output_path}.")
    except IOError as e:
        print(f"Error writing inferred sentiment lexicon to {output_path}: {e}")

def main():
    parser = argparse.ArgumentParser(description="Create a sentiment lexicon for a corpus by inferring from a base lexicon.")
    parser.add_argument(
        "--corpus_dir",
        type=str,
        default=CORPUS_DIR_DEFAULT,
        help="Path to the corpus directory to scan."
    )
    parser.add_argument(
        "--sentiments_cat_path",
        type=str,
        default=SENTIMENTS_CAT_PATH_DEFAULT,
        help="Path to the base sentiments.cat file."
    )
    parser.add_argument(
        "--output_path",
        type=str,
        default=OUTPUT_PATH_DEFAULT,
        help="Path to save the new corpus_sentiments.cat file."
    )
    parser.add_argument(
        "--max_distance_threshold",
        type=int,
        default=1, # Default to a very strict threshold (1 edit distance)
        help="Maximum Levenshtein distance to infer sentiment from a base word."
    )
    args = parser.parse_args()

    # --- Load Base Sentiment Lexicon ---
    base_lexicon = load_base_sentiment_lexicon(args.sentiments_cat_path)
    if not base_lexicon:
        print("Cannot proceed without a base sentiment lexicon.")
        sys.exit(1)

    # --- Scan Corpus Words ---
    corpus_words = scan_corpus_words(args.corpus_dir)
    if not corpus_words:
        print("No words found in the corpus to infer sentiments for.")
        sys.exit(1)

    # --- Infer Corpus Sentiments ---
    inferred_lexicon = infer_corpus_sentiments(corpus_words, base_lexicon, args.max_distance_threshold)

    # --- Save Corpus Sentiments ---
    save_corpus_sentiments(inferred_lexicon, args.output_path)

if __name__ == "__main__":
    main()
