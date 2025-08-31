import os
import json
import argparse
import re
import numpy as np

def load_dictionary(dict_path):
    """
    Loads a JSON dictionary file.
    Returns a set of words for fast lookups.
    """
    if not dict_path or not os.path.exists(dict_path):
        print("Warning: Dictionary file not found or not provided. Using a basic alphabetic check for vocabulary.")
        return None
    try:
        with open(dict_path, 'r', encoding='utf-8') as f:
            words = json.load(f)
            # Assuming the json is a list of words or a dict with words as keys
            if isinstance(words, dict):
                return set(words.keys())
            else:
                return set(words)
    except (json.JSONDecodeError, IOError) as e:
        print(f"Error loading dictionary file: {e}. Proceeding without a dictionary.")
        return None

def tokenize(text):
    """
    A simple tokenizer that splits text into words.
    """
    # Normalize text to lowercase and split by non-alphanumeric characters
    return re.findall(r'\b\w+\b', text.lower())

def evaluate_vocabulary(tokens, dictionary):
    """
    Evaluates the vocabulary of a list of tokens.
    Returns the percentage of words found in the dictionary.
    """
    if not tokens:
        return 0.0
    
    if dictionary:
        valid_words = sum(1 for token in tokens if token in dictionary)
    else:
        # Fallback: consider any word with only alphabetic characters as valid
        valid_words = sum(1 for token in tokens if token.isalpha())
        
    return (valid_words / len(tokens)) * 100

def evaluate_structure(text):
    """
    Evaluates the structural complexity of the text.
    Returns a score based on sentence length, punctuation, and paragraphing.
    """
    # 1. Sentence Score (based on average length)
    sentences = re.split(r'[.?!]', text)
    sentences = [s for s in sentences if s.strip()]
    if not sentences:
        return 0.0
        
    avg_sentence_length = np.mean([len(tokenize(s)) for s in sentences])
    # Score is higher for lengths between 10 and 30 words, penalizing very short or long sentences.
    sentence_score = np.exp(-((avg_sentence_length - 20) ** 2) / (2 * 10 ** 2)) * 100

    # 2. Punctuation Diversity Score
    punctuations = re.findall(r'[,;:"\"]', text)
    punctuation_diversity = len(set(punctuations))
    # Score based on the number of unique punctuation types used.
    punc_score = min(punctuation_diversity / 5.0, 1.0) * 100

    # 3. Paragraph Score
    paragraphs = [p for p in text.split('\n\n') if p.strip()]
    num_paragraphs = len(paragraphs)
    # We want to avoid a single block of text or excessive newlines.
    # Score is higher if there is more than one paragraph.
    para_score = (1 - 1 / (num_paragraphs + 1)) * 100

    # Combine scores (weights can be tuned)
    structure_score = (0.5 * sentence_score) + (0.3 * punc_score) + (0.2 * para_score)
    return structure_score

def evaluate_corpus(corpus_dir, dictionary):
    """
    Iterates through a corpus directory, evaluates each file, and returns a ranked list.
    """
    file_scores = []
    
    for root, _, files in os.walk(corpus_dir):
        for file in files:
            # For now, let's assume all files are text files.
            # A more robust version could check for extensions like .txt, .md, etc.
            file_path = os.path.join(root, file)
            try:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                
                if not content.strip():
                    continue

                tokens = tokenize(content)
                
                # Calculate scores
                vocab_score = evaluate_vocabulary(tokens, dictionary)
                struct_score = evaluate_structure(content)
                
                # Final "usefulness" score (weights can be tuned)
                usefulness = (0.6 * vocab_score) + (0.4 * struct_score)
                
                file_scores.append({
                    "file": file_path,
                    "usefulness": usefulness,
                    "vocab_score": vocab_score,
                    "structure_score": struct_score
                })

            except Exception as e:
                print(f"Could not process file {file_path}: {e}")

    # Sort files from highest to lowest usefulness
    ranked_files = sorted(file_scores, key=lambda x: x['usefulness'], reverse=True)
    return ranked_files

def main():
    """
    Main function to parse arguments and run the corpus evaluation.
    """
    parser = argparse.ArgumentParser(description="Evaluate and rank files in a corpus for training usefulness.")
    parser.add_argument(
        "corpus_dir",
        type=str,
        help="The directory containing the corpus files to evaluate."
    )
    parser.add_argument(
        "--dict_path",
        type=str,
        default=None,
        help="Path to a JSON file containing an English dictionary word list."
    )
    
    args = parser.parse_args()
    
    if not os.path.isdir(args.corpus_dir):
        print(f"Error: Corpus directory not found at '{args.corpus_dir}'")
        return
        
    dictionary = load_dictionary(args.dict_path)
    
    print(f"Evaluating corpus in: {args.corpus_dir}")
    report = evaluate_corpus(args.corpus_dir, dictionary)
    
    print("\n--- Corpus Usefulness Report ---")
    print(f"{ 'Rank':<5} | {'Usefulness Score':<20} | {'File Path':<}")
    print("-" * 80)
    
    for i, result in enumerate(report):
        print(f"{i+1:<5} | {result['usefulness']:<20.2f} | {result['file']}")
        
if __name__ == "__main__":
    main()
