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
from quanta_tissu.tisslm.core.utils import convert_to_utf8

# --- Configuration ---
CORPUS_DIR_DEFAULT = os.path.join(project_root, "corpus")
SENTIMENTS_CAT_PATH_DEFAULT = os.path.join(project_root, "data", "sentiments.cat")
OUTPUT_PATH_DEFAULT = os.path.join(project_root, "data", "corpus_sentiments.cat")

def load_base_sentiment_lexicon(file_path):
    """Loads the base sentiment lexicon from a .cat file."""
    lexicon = {}
    current_category = None
    category_scores = {
        "NEGATIVE": -1.0,
        "POSITIVE": 1.0,
        "TO_BE_IGNORED": 0.0 # Neutral or ignored
    }

    if not os.path.exists(file_path):
        print(f"Error: Base sentiment lexicon not found at {file_path}.")
        return lexicon
    try:
        with open(file_path, 'rb') as f: # Open in binary mode
            for line_bytes in f:
                line = convert_to_utf8(line_bytes).strip() # Convert to UTF-8 and then strip

                if not line or line.startswith('#'):
                    continue

                # Check for top-level categories (all uppercase, no leading space/tab, and does not end with '(1)')
                # The original file uses no indentation for top-level categories.
                if line.isupper() and not line.startswith(' ') and not line.startswith('\t') and not line.endswith('(1)'):
                    current_category = line
                    continue

                # Check for words (ends with '(1)')
                # Words are indented with tabs or spaces, but the key is the (1) suffix.
                if line.endswith('(1)'):
                    # Extract word part, handling potential leading tabs/spaces and @ symbols
                    # Split by space, take the first part, convert to lowercase, remove * and @
                    word_part = line.strip().split(' ')[0].lower()
                    word = word_part.replace('*', '').replace('@', '')

                    if current_category in category_scores:
                        lexicon[word] = category_scores[current_category]
                    else:
                        # Default to neutral if category not explicitly handled or no category yet
                        lexicon[word] = 0.0
                else:
                    pass # Line did not match any parsing rule, silently ignore


        print(f"Loaded {len(lexicon)} words from base sentiment lexicon.")
    except IOError as e:
        print(f"Error reading base sentiment lexicon {file_path}: {e}")
    except Exception as e:
        print(f"An unexpected error occurred during lexicon loading: {e}")
    return lexicon


def scan_corpus_words(corpus_dir):
    """
    Scans all text files in a corpus directory and returns a Counter of word frequencies.
    """
    word_counts = Counter()
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
                        word_counts.update(words)
                except Exception as e:
                    print(f"Warning: Could not process file {file_path}: {e}")
    print(f"Found {len(word_counts)} unique words in corpus.")
    return word_counts

def infer_corpus_sentiments(corpus_words_counts, base_lexicon, max_distance_threshold, limit_corpus_words=100, limit_base_lexicon=100):
    """
    Infers sentiment for a limited number of corpus words based on a limited base lexicon and Levenshtein distance.
    """
    inferred_lexicon = {}

    # Get top N most common corpus words
    top_corpus_words = [word for word, count in corpus_words_counts.most_common(limit_corpus_words)]

    # Limit base lexicon words
    limited_base_lexicon_words = list(base_lexicon.keys())[:limit_base_lexicon]

    print(f"Inferring sentiments for top {len(top_corpus_words)} corpus words against {len(limited_base_lexicon_words)} base lexicon words...")
    for i, word in enumerate(top_corpus_words):
        if i % 10 == 0: # More frequent progress update for smaller set
            print(f"  Processed {i}/{len(top_corpus_words)} words...")

        if word in base_lexicon: # Check against full base_lexicon for exact matches
            inferred_lexicon[word] = base_lexicon[word]
        else:
            min_distance = float('inf')
            closest_base_word = None

            for base_word in limited_base_lexicon_words: # Only iterate over limited base lexicon
                distance = levenshtein_distance(word, base_word)
                if distance < min_distance:
                    min_distance = distance
                    closest_base_word = base_word
                if distance == 0: # Exact match found in limited_base_lexicon_words
                    break
            
            if closest_base_word and min_distance <= max_distance_threshold:
                inferred_lexicon[word] = base_lexicon[closest_base_word] # Use score from full base_lexicon
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
        "---sentiments_cat_path",
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
    # New arguments for limiting words
    parser.add_argument(
        "--limit_corpus_words",
        type=int,
        default=100,
        help="Limit the number of most frequent corpus words to infer sentiments for."
    )
    parser.add_argument(
        "--limit_base_lexicon",
        type=int,
        default=100,
        help="Limit the number of base lexicon words to use for inference."
    )
    args = parser.parse_args()

    # --- Load Base Sentiment Lexicon ---
    base_lexicon = load_base_sentiment_lexicon(args.sentiments_cat_path)
    if not base_lexicon:
        print("Cannot proceed without a base sentiment lexicon.")
        sys.exit(1)

    # --- Scan Corpus Words ---
    corpus_word_counts = scan_corpus_words(args.corpus_dir)
    if not corpus_word_counts:
        print("No words found in the corpus to infer sentiments for.")
        sys.exit(1)

    # --- Infer Corpus Sentiments ---
    inferred_lexicon = infer_corpus_sentiments(
        corpus_word_counts,
        base_lexicon,
        args.max_distance_threshold,
        limit_corpus_words=args.limit_corpus_words,
        limit_base_lexicon=args.limit_base_lexicon
    )

    # --- Save Corpus Sentiments ---
    save_corpus_sentiments(inferred_lexicon, args.output_path)

if __name__ == "__main__":
    main()
