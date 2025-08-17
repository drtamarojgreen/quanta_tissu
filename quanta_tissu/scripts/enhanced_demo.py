import sys
import os
import numpy as np

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from quanta_tissu.tisslm.model import QuantaTissu
from quanta_tissu.tisslm.tokenizer import Tokenizer, detokenize
from quanta_tissu.tisslm.config import model_config

def demonstrate_enhanced_functionality():
    """
    Comprehensive demonstration of the enhanced QuantaTissu functionality
    including the improved tokenizer, batched inference, and self-updating knowledge base.
    """
    print("=" * 60)
    print("QuantaTissu Enhanced Functionality Demonstration")
    print("=" * 60)
    
    # --- Setup ---
    np.random.seed(42)
    model = QuantaTissu(model_config)
    tokenizer = Tokenizer()
    
    print("\n1. Enhanced Tokenizer Demonstration")
    print("-" * 40)
    
    # Test the new tokenizer class
    test_text = "hello world test"
    tokens = tokenizer.tokenize(test_text)
    reconstructed = tokenizer.detokenize(tokens)
    
    print(f"Original text: '{test_text}'")
    print(f"Tokenized: {tokens}")
    print(f"Reconstructed: '{reconstructed}'")
    print(f"Vocabulary size: {tokenizer.get_vocab_size()}")
    
    # --- Knowledge Base Enhancement Demo ---
    print("\n2. Enhanced Knowledge Base Demonstration")
    print("-" * 40)
    
    # Add documents with metadata
    model.knowledge_base.add_document("cats are great pets", metadata={'source': 'animal_facts'})
    model.knowledge_base.add_document("dogs are loyal companions", metadata={'source': 'animal_facts'})
    model.knowledge_base.add_document("hello world test", metadata={'source': 'programming'})
    
    # Show knowledge base stats
    stats = model.knowledge_base.get_knowledge_stats()
    print(f"\nKnowledge Base Stats:")
    for key, value in stats.items():
        print(f"  {key}: {value}")
    
    # --- Retrieval and Feedback Demo ---
    print("\n3. Retrieval and Feedback Demonstration")
    print("-" * 40)
    
    query = "pets"
    retrieved_docs = model.knowledge_base.retrieve(query, k=2)
    print(f"Query: '{query}'")
    print(f"Retrieved documents: {retrieved_docs}")
    
    # Simulate user feedback
    model.knowledge_base.add_feedback(query, retrieved_docs, feedback_score=4, 
                                    feedback_text="Good results for pet query")
    
    # --- Self-Updating Demo ---
    print("\n4. Self-Updating Mechanism Demonstration")
    print("-" * 40)
    
    # Simulate a model interaction with correction
    user_query = "what are cats"
    model_response = "unknown"
    user_correction = "cats are feline animals"
    
    model.knowledge_base.self_update_from_interaction(user_query, model_response, user_correction)
    
    # Show updated stats
    updated_stats = model.knowledge_base.get_knowledge_stats()
    print(f"\nUpdated Knowledge Base Stats:")
    for key, value in updated_stats.items():
        print(f"  {key}: {value}")
    
    # --- Batched Inference Demo ---
    print("\n5. Batched Inference Demonstration")
    print("-" * 40)
    
    # Test single sequence
    single_tokens = tokenizer.tokenize("hello world")
    single_logits = model.forward(single_tokens)
    print(f"Single sequence shape: {single_tokens.shape} -> {single_logits.shape}")
    
    # Test batched sequences (simulate by creating a batch)
    batch_tokens = np.array([
        np.pad(tokenizer.tokenize("hello"), (0, 2), constant_values=1),  # pad with <pad> token
        np.pad(tokenizer.tokenize("world test"), (0, 1), constant_values=1)
    ])
    batch_logits = model.forward(batch_tokens)
    print(f"Batch shape: {batch_tokens.shape} -> {batch_logits.shape}")
    
    # --- Advanced Generation Demo ---
    print("\n6. Advanced Generation Methods Demonstration")
    print("-" * 40)
    
    test_prompt = "hello"
    test_tokens = tokenizer.tokenize(test_prompt)
    
    # Test different generation methods
    methods = [
        ("greedy", {}),
        ("top_k", {"top_k": 3}),
        ("nucleus", {"top_p": 0.8}),
        ("random", {"temperature": 1.5})
    ]
    
    for method, kwargs in methods:
        try:
            next_token = model.predict(test_tokens, method=method, **kwargs)
            next_word = tokenizer.get_token(next_token)
            print(f"{method:10} -> '{next_word}' (token {next_token})")
        except Exception as e:
            print(f"{method:10} -> Error: {e}")
    
    # --- Knowledge-Based Generation Demo ---
    print("\n7. Knowledge-Based Generation Demonstration")
    print("-" * 40)
    
    kb_prompts = ["cats", "hello", "test"]
    
    for prompt in kb_prompts:
        print(f"\nPrompt: '{prompt}'")
        try:
            next_token = model.generate_with_kb(prompt, generation_method="greedy")
            if next_token is not None:
                next_word = tokenizer.get_token(next_token)
                print(f"Generated: '{next_word}'")
            else:
                print("Generation failed")
        except Exception as e:
            print(f"Error: {e}")
    
    print("\n" + "=" * 60)
    print("Demonstration Complete!")
    print("=" * 60)

if __name__ == "__main__":
    demonstrate_enhanced_functionality()
